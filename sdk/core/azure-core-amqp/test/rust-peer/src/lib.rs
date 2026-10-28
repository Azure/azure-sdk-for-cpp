// Copyright (c) Microsoft Corporation. Licensed under the MIT License.
// Test-only AMQP peer. No production target links this crate.
// cspell: words amqp tokio fe2o3
use fe2o3_amqp::{
    acceptor::{ConnectionAcceptor, LinkAcceptor, LinkEndpoint, SessionAcceptor},
    types::messaging::{ApplicationProperties, Body, Message, Properties},
};
use fe2o3_amqp_types::primitives::{Array, OrderedMap, Timestamp, Value};
use std::{
    sync::{
        atomic::{AtomicBool, Ordering},
        Arc, Condvar, Mutex,
    },
    time::Duration,
};

pub const HOLD_QUERY: u32 = 1;
pub const SERVICE_ERROR: u32 = 2;
pub const REQUEST: u32 = 1;

struct State {
    mode: u32,
    observed: Mutex<u32>,
    changed: Condvar,
    resumed: AtomicBool,
}
impl State {
    fn signal(&self, event: u32) {
        *self.observed.lock().unwrap() |= event;
        self.changed.notify_all();
    }
}
pub struct CleanupPeer {
    // Runtime destruction joins the helper tasks, including deliberately stalled
    // peers. This is fixture cleanup, not proof of SDK engine termination.
    _runtime: tokio::runtime::Runtime,
    state: Arc<State>,
    port: u16,
}
impl CleanupPeer {
    pub fn start(mode: u32) -> Self {
        let runtime = tokio::runtime::Builder::new_multi_thread()
            .worker_threads(2)
            .enable_all()
            .build()
            .unwrap();
        let state = Arc::new(State {
            mode,
            observed: Mutex::new(0),
            changed: Condvar::new(),
            resumed: AtomicBool::new(false),
        });
        let listener = runtime
            .block_on(tokio::net::TcpListener::bind("127.0.0.1:0"))
            .unwrap();
        let port = listener.local_addr().unwrap().port();
        let shared = state.clone();
        runtime.spawn(async move {
            while let Ok((tcp, _)) = listener.accept().await {
                let state = shared.clone();
                tokio::spawn(async move {
                    let Ok(mut connection) = ConnectionAcceptor::builder()
                        .container_id("cleanup-peer")
                        .sasl_acceptor(fe2o3_amqp::acceptor::SaslAnonymousMechanism::new())
                        .build()
                        .accept(tcp)
                        .await
                    else {
                        return;
                    };
                    while let Some(incoming) = connection.next_incoming_session().await {
                        let Ok(session) = SessionAcceptor::new()
                            .accept_incoming_session(incoming, &mut connection)
                            .await
                        else {
                            break;
                        };
                        tokio::spawn(serve_session(session, state.clone()));
                    }
                });
            }
        });
        Self {
            _runtime: runtime,
            state,
            port,
        }
    }
    pub fn port(&self) -> u16 {
        self.port
    }
    pub fn wait(&self, event: u32, timeout: Duration) -> bool {
        let observed = self.state.observed.lock().unwrap();
        let (observed, _) = self
            .state
            .changed
            .wait_timeout_while(observed, timeout, |flags| *flags & event == 0)
            .unwrap();
        *observed & event != 0
    }
    pub fn resume(&self) {
        self.state.resumed.store(true, Ordering::SeqCst);
    }
}

async fn serve_session(
    mut session: fe2o3_amqp::acceptor::ListenerSessionHandle,
    state: Arc<State>,
) {
    let acceptor = LinkAcceptor::new();
    let mut pending_replies = None;
    while let Some(attach) = session.next_incoming_attach().await {
        let Ok(link) = acceptor.accept_incoming_attach(attach, &mut session).await else {
            continue;
        };
        match link {
            LinkEndpoint::Receiver(mut receiver) => {
                let (responses, replies) = tokio::sync::mpsc::channel::<Message<Body<Value>>>(16);
                pending_replies = Some(replies);
                let state = state.clone();
                tokio::spawn(async move {
                    while let Ok(delivery) = receiver.recv::<Body<Value>>().await {
                        let target = receiver
                            .target()
                            .as_ref()
                            .and_then(|t| t.address.as_ref())
                            .map(|s| s.as_str())
                            .unwrap_or("");
                        let is_management = target == "$management";
                        let is_cbs = target == "$cbs";
                        let _ = receiver.accept(&delivery).await;
                        if !is_management && !is_cbs {
                            continue;
                        }
                        if is_management {
                            state.signal(REQUEST);
                        }
                        if is_management
                            && state.mode & HOLD_QUERY != 0
                            && !state.resumed.load(Ordering::SeqCst)
                        {
                            continue;
                        }
                        let correlation_id = delivery
                            .message()
                            .properties
                            .as_ref()
                            .and_then(|p| p.message_id.clone());
                        let mut body = OrderedMap::<String, Value>::new();
                        body.insert("name".into(), "hub".into());
                        body.insert("created_at".into(), Value::Timestamp(Timestamp::from(0)));
                        body.insert(
                            "partition_ids".into(),
                            Value::Array(Array::from(vec![
                                Value::String("0".to_string()),
                                Value::String("1".to_string()),
                            ])),
                        );
                        body.insert("partition".into(), "0".into());
                        body.insert("begin_sequence_number".into(), Value::Long(0));
                        body.insert("last_enqueued_sequence_number".into(), Value::Long(1));
                        body.insert("last_enqueued_offset".into(), "1".into());
                        body.insert(
                            "last_enqueued_time_utc".into(),
                            Value::Timestamp(Timestamp::from(0)),
                        );
                        body.insert("is_partition_empty".into(), false.into());
                        let status = if is_cbs {
                            202i32
                        } else if state.mode & SERVICE_ERROR != 0
                            && !state.resumed.load(Ordering::SeqCst)
                        {
                            500i32
                        } else {
                            200i32
                        };
                        let message = Message::builder()
                            .properties(Properties {
                                correlation_id,
                                ..Default::default()
                            })
                            .application_properties(
                                ApplicationProperties::builder()
                                    .insert("statusCode", status)
                                    .insert("statusDescription", "cleanup peer response")
                                    .build(),
                            )
                            .value(Value::Map(
                                body.into_iter()
                                    .map(|(k, v)| (Value::String(k), v))
                                    .collect(),
                            ))
                            .build();
                        if responses.send(message.map_body(Body::Value)).await.is_err() {
                            break;
                        }
                    }
                });
            }
            LinkEndpoint::Sender(mut sender) => {
                let Some(mut replies) = pending_replies.take() else {
                    break;
                };
                // Pair each response link with the preceding request link.
                tokio::spawn(async move {
                    let mut responses_closed = false;
                    loop {
                        tokio::select! {
                            _ = sender.on_detach() => { let _ = sender.close().await; break; },
                            response = replies.recv(), if !responses_closed => {
                                let Some(response) = response else { responses_closed = true; continue; };
                                if sender.send(response).await.is_err() { break; }
                            }
                        }
                    }
                });
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn cleanup_peer_start(mode: u32) -> *mut CleanupPeer {
    Box::into_raw(Box::new(CleanupPeer::start(mode)))
}
/// # Safety
/// `peer` must be a live pointer returned by cleanup_peer_start.
#[no_mangle]
pub unsafe extern "C" fn cleanup_peer_port(peer: *const CleanupPeer) -> u16 {
    (*peer).port()
}
/// # Safety
/// `peer` must be live for the duration of this call.
#[no_mangle]
pub unsafe extern "C" fn cleanup_peer_wait(
    peer: *const CleanupPeer,
    event: u32,
    timeout_ms: u64,
) -> bool {
    (*peer).wait(event, Duration::from_millis(timeout_ms))
}
/// # Safety
/// `peer` must be live for the duration of this call.
#[no_mangle]
pub unsafe extern "C" fn cleanup_peer_resume(peer: *const CleanupPeer) {
    (*peer).resume();
}
/// # Safety
/// `peer` must be uniquely owned and consumed exactly once.
#[no_mangle]
pub unsafe extern "C" fn cleanup_peer_stop(peer: *mut CleanupPeer) {
    drop(Box::from_raw(peer));
}
