# Event Hubs cleanup regression tests

These tests cover producer cleanup after partition discovery fails and cleanup while a properties query is running. They verify that management clients, sessions, and connection polling references are released safely, including when initialization or shutdown throws.

The tests use local loopback listeners with the uAMQP and Rust backends. They do not require Azure credentials, a live broker, or the HTTP test proxy. See [CONTRIBUTING.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md) for compiler and dependency setup.

## Build and run with uAMQP

Run from the repository root, adding any toolchain or platform options required by your development environment:

```sh
cmake -S . -B build/cleanup \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -D AZURE_EVENTHUBS_CTEST_TIMEOUT=60 \
  -DDISABLE_RUST_IN_BUILD=ON

cmake --build build/cleanup --config Debug --target \
  azure-core-amqp-tests azure-messaging-eventhubs-test

ctest --test-dir build/cleanup -C Debug -N \
  -R 'SessionCleanupDeathTest|PropertiesCleanupDeathTest'

ctest --test-dir build/cleanup -C Debug \
  -R 'SessionCleanupDeathTest|PropertiesCleanupDeathTest' \
  --output-on-failure --timeout 60
```

The configure option sets the Event Hubs test timeout to 60 seconds; CTest's `--timeout` alone does not override an explicit per-test timeout.

The discovery command should list **21 cases**: four session cases, nine properties cleanup cases, and eight parameterized concurrency cases. These two test files are guarded by `ENABLE_UAMQP`. The Rust backend uses the separate cases described below; a successful run with no matching tests does not validate this cleanup behavior.

To repeat just the concurrency cases five times:

```sh
ctest --test-dir build/cleanup -C Debug \
  -R 'ConcurrentPropertiesCleanupDeathTest' \
  --repeat until-fail:5 --output-on-failure --timeout 60
```

## Build and run with Rust

```sh
cmake -S . -B build/cleanup-rust \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DAZURE_EVENTHUBS_CTEST_TIMEOUT=60 \
  -DDISABLE_RUST_IN_BUILD=OFF
cmake --build build/cleanup-rust --config Debug --target azure-messaging-eventhubs-test
ctest --test-dir build/cleanup-rust -C Debug \
  -R 'Rust.*PropertiesCleanupDeathTest' --output-on-failure --timeout 60
cargo test -p rust_wrapper call_context -- --test-threads=1
```

This selects **13 C++ cases** and **six Rust context cases**. The C++ tests link a test-only Rust AMQP peer built from the existing fe2o3 dependency. The peer acknowledges receipt of management requests and withholds their responses, so cancellation occurs after a real request reaches the server. A 15-second child-process watchdog reports a blocked query or close as a failure.

Source: [rust_properties_cleanup_test.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/test/ut/rust_properties_cleanup_test.cpp).

| Cases | Behavior checked |
| --- | --- |
| `CloseDuringPendingQuery` (four) | Both getters release their operation lock when close cancels the child context; caller cancellation state is preserved, repeated close succeeds, and stale references reject queries. |
| `ConsumerCloseDuringPendingQuery` (four) | Public consumer close completes with either getter pending, including when the close deadline has already expired. The original query context has no deadline and remains uncancelled. |
| `CancelledPropertiesThenRediscover`, `CancelledPartitionThenRediscover` | Caller cancellation interrupts the query; subsequent public producer queries succeed with fresh links. |
| `QueryDeadlineThenRediscover` | A query deadline interrupts the wait; a later query succeeds. |
| `ServiceErrorIsExceptionThenRediscover` | A real service error produces an exception instead of converting a null Rust value; a later query succeeds. |
| `DirectManagementFailureRequiresReopen` | A failed management client rejects reuse until close and reopen. |

The Rust context tests cover cancellation before and during a wait, deadlines, successful results, dropping the pending future exactly once, and teardown that runs despite cancellation. These changes cover management calls after initialization; they do not establish cancellation of pending session begin, authentication, or management attach, or forced termination of dependency background tasks.

## Session ownership and shutdown

Source: [session_cleanup_tests.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core-amqp/test/ut/session_cleanup_tests.cpp), fixture `SessionCleanupDeathTest`.

| Test | Behavior checked |
| --- | --- |
| `FailedEndReleasesLastOwner` | A native shutdown failure still throws. A management client keeps the session alive until its last owner releases it; the session and connection then expire and polling becomes idle. |
| `FailedEndWithErrorReleasesLastOwner` | The same ownership guarantees hold for the `End` overload that accepts an AMQP error condition and description. |
| `FailedEndCanBeRetried` | A failed `End` can be retried successfully through the other overload. A further `End` fails rather than releasing polling ownership twice. |
| `MissingEndStillAsserts` | Destroying a begun session without ever calling `End` still terminates the child process. The existing lifecycle contract remains enforced. |

## Properties initialization and producer cleanup

Source: [properties_cleanup_test.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/test/ut/properties_cleanup_test.cpp), fixture `PropertiesCleanupDeathTest`.

| Test | Behavior checked |
| --- | --- |
| `CancelledQueryThenRediscover` | Caller cancellation interrupts a query, and both getters succeed on subsequent queries with fresh management links. |
| `SuccessfulDiscoveryAndRepeatedClose` | Hub and partition properties are returned correctly, and repeated close ends the session exactly once. |
| `ManagementCloseFailureStillEndsSession` | A canceled management close logs its failure, but session cleanup still runs exactly once. |
| `CreationFailureThenRepeatedClose` | Management creation throws after session begin. Retrying creation preserves the original error and does not acquire another polling reference; repeated close releases ownership. |
| `CreationFailureThenDestruction` | Destruction performs the same partial-initialization cleanup without an explicit close. |
| `CreationFailureAndSessionEndFailure` | Management creation and native session shutdown both fail; final destruction still releases the owned resources. |
| `OpenFailureThenRepeatedClose` | Management open fails after assignment; repeated close safely cleans the partially initialized client. |
| `OpenFailureAndSessionEndFailure` | An open failure followed by a native session shutdown failure does not cause a destruction assertion. |
| `ProducerDiscoveryFailureThenRepeatedClose` | A mock management endpoint returns a service error. The public producer reports the discovery failure and survives repeated close and destruction. |

## Concurrent queries and cleanup

The same source contains `ConcurrentPropertiesCleanupDeathTest`, instantiated as `PropertiesAndPartitions`. Each test runs for both getters and with caller cancellation either absent or already requested: two tests times two getters times two cancellation states gives eight cases.

| Test | Synchronization and expected outcome |
| --- | --- |
| `CloseDuringPendingQuery` | The mock endpoint signals receipt but withholds the response. The query has no deadline. Close must cancel its child operation context and finish without waiting indefinitely for the server. |
| `CloseBeforeManagementSend` | A synchronous log callback pauses execution inside `ExecuteOperation`, before sending. While paused, close must remain pending and the session-end counter must stay zero. After release, the query and close must finish safely. |

Both tests verify that cleanup ends the session once, preserves the caller's cancellation state, rejects later queries through the closed properties-client reference, and leaves global polling idle. The parameter tuple is `(partitionProperties, cancelCaller)`: indices 0 through 3 correspond to `(false, false)`, `(false, true)`, `(true, false)`, and `(true, true)`.

The second test depends on the management log messages containing `ManagementClient::ExecuteOperation:` and `Create Queue for request.`. If those diagnostics change, update the gate as well. The gate uses a bounded wait so a missing signal fails the child instead of hanging the suite.

## Why these are process tests

Except for `MissingEndStillAsserts`, the tests use `EXPECT_EXIT` and require a zero child exit status. An exception assertion alone would miss the original crash: a later destructor can abort after the exception has already been caught. The fixtures use Google Test's `threadsafe` death-test style to re-execute the child without inheriting polling-thread mutexes.

The [cleanup listener](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core-amqp/test/ut/cleanup_test_listener.hpp) accepts TCP connection attempts without creating a peer AMQP session. This isolates injected creation and session-end failures from mock-server teardown. Discovery and concurrency cases use the AMQP mock server because they need management links and requests. Weak ownership checks, an atomic session-end counter, and `GlobalStateHolder::AssertIdle()` check resource lifetime in addition to process survival.

The concurrency gate reproduced process termination and an AddressSanitizer heap-use-after-free before synchronization was added. For Google Test 1.12 on Linux ARM64 under AddressSanitizer, set `GTEST_DEATH_TEST_USE_FORK=1` to avoid the small clone-child stack overflowing before the SDK code runs.

These local tests establish SDK cleanup behavior. They do not replace Windows teardown validation, live service tests, or the AUB discovery-failure recovery workload. The Blob Checkpoint Store recordings refreshed alongside this fix exercise separate, existing HTTP playback tests.
