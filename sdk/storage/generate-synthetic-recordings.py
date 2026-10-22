#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

"""Generate explicitly synthetic, offline-only Storage test-proxy fixtures."""

import argparse
import base64
import json
from pathlib import Path
from urllib.parse import urlencode, urlsplit


SERVER = "Azure-Storage-Cpp-Synthetic-Test-Server"
TENANT = "11111111-1111-1111-1111-111111111111"
INVALID_TENANT = "00000000-0000-0000-0000-000000000000"
OBJECT_ID = "33333333-3333-3333-3333-333333333333"
KEY_OBJECT_ID = "22222222-2222-2222-2222-222222222222"
LEASE = "44444444-4444-4444-4444-444444444444"
WRONG_LEASE = "55555555-5555-5555-5555-555555555555"
START = "2020-01-01T00:00:00Z"
EXPIRY = "2099-01-01T00:00:00Z"
VERSION = "2026-10-06"
DATE = "Thu, 22 Oct 2026 00:00:00 GMT"
KEY = base64.b64encode(bytes(32)).decode()
TOKEN = "Sanitized"


def url(service, path="", **query):
    result = f"https://Sanitized.{service}.core.windows.net/{path}"
    return result + ("?" + urlencode(query, safe=":,") if query else "")


def entry(method, uri, status=200, body="", request_headers=None, headers=None):
    host = urlsplit(uri).hostname
    storage = ".core.windows.net" in host
    recorded_headers = {"User-Agent": "synthetic", "x-ms-client-request-id": "Sanitized"}
    if storage:
        recorded_headers.update({
            "x-ms-date": DATE,
            "x-ms-version": (
                "2026-06-06" if ".dfs." in host else
                "2026-04-06" if ".queue." in host else VERSION
            ),
        })
        if status != 401:
            recorded_headers["Authorization"] = "Sanitized"
    if method in ["PUT", "POST"]:
        recorded_headers["Content-Length"] = "0"
    if not storage:
        recorded_headers["Content-Type"] = "application/x-www-form-urlencoded"
    elif "comp=userdelegationkey" in uri:
        recorded_headers["Content-Type"] = "application/xml; charset=UTF-8"
    if ".file." in host and method == "HEAD":
        recorded_headers["x-ms-file-request-intent"] = "backup"
    recorded_headers.update(request_headers or {})
    response_headers = {
        "Server": SERVER,
        "Date": DATE,
        "Content-Length": str(len(body.encode())),
        "x-ms-request-id": "00000000-0000-0000-0000-000000000001",
        "x-ms-version": VERSION,
    }
    response_headers.update(headers or {})
    if method != "HEAD":
        response_headers["Content-Length"] = str(len(body.encode()))
    return {
        "RequestUri": uri,
        "RequestMethod": method,
        "RequestHeaders": recorded_headers,
        "RequestBody": None,
        "StatusCode": status,
        "ResponseHeaders": response_headers,
        "ResponseBody": (
            base64.b64encode(body.encode()).decode()
            if body and response_headers.get("Content-Type") == "application/octet-stream"
            else body
        ),
    }


def properties(blob_type="BlockBlob"):
    return {
        "ETag": '"synthetic-etag"',
        "Last-Modified": DATE,
        "Content-Type": "application/octet-stream",
        "Content-Length": "9",
        "x-ms-blob-type": blob_type,
        "x-ms-lease-status": "unlocked",
        "x-ms-lease-state": "available",
        "x-ms-has-immutability-policy": "false",
        "x-ms-has-legal-hold": "false",
        "x-ms-server-encrypted": "true",
        "x-ms-creation-time": DATE,
        "x-ms-file-attributes": "Archive",
        "x-ms-file-creation-time": START,
        "x-ms-file-last-write-time": START,
        "x-ms-file-change-time": START,
        "x-ms-file-permission-key": "synthetic-permission-key",
        "x-ms-file-id": "1",
        "x-ms-file-parent-id": "0",
    }


def error(method, uri, code, status=403, request_headers=None):
    return entry(
        method,
        uri,
        status,
        "" if method == "HEAD" else (
            f"<Error><Code>{code}</Code><Message>Synthetic {code} response.</Message></Error>"
        ),
        request_headers,
        {"x-ms-error-code": code, **(
            {"Content-Type": "application/xml"} if method != "HEAD" else {}
        )},
    )


def token():
    return entry(
        "POST",
        f"https://Sanitized.microsoftonline.com/{TENANT}/oauth2/v2.0/token",
        body=json.dumps({"access_token": TOKEN, "expires_in": 3600, "token_type": "Bearer"}),
        headers={"Content-Type": "application/json"},
    )


def delegation_key(service, tenant):
    body = (
        "<UserDelegationKey>"
        f"<SignedOid>{KEY_OBJECT_ID}</SignedOid><SignedTid>{INVALID_TENANT}</SignedTid>"
        f"<SignedStart>{START}</SignedStart><SignedExpiry>{EXPIRY}</SignedExpiry>"
        f"<SignedService>{service}</SignedService><SignedVersion>{VERSION}</SignedVersion>"
        f"<SignedDelegatedUserTid>{tenant}</SignedDelegatedUserTid><Value>{KEY}</Value>"
        "</UserDelegationKey>"
    )
    return entry(
        "POST",
        url({"b": "blob", "f": "file", "q": "queue"}[service],
            restype="service", comp="userdelegationkey"),
        body=body,
        headers={"Content-Type": "application/xml"},
    )


def cross_tenant(service):
    key_service = {"blob": "b", "datalake": "b", "file": "f", "queue": "q"}[service]
    resource_service = {"datalake": "blob"}.get(service, service)
    path = {"blob": "container/blob", "datalake": "filesystem/file",
            "file": "share/file", "queue": "queue"}[service]
    entries = [token()]
    for tenant in [TENANT, INVALID_TENANT]:
        entries.append(delegation_key(key_service, tenant))
        query = dict(
            sp={"blob": "racwdxyti", "datalake": "racwdltmeop", "file": "rcwd",
                "queue": "raup"}[service],
            spr="https,http", skoid=KEY_OBJECT_ID, sktid=INVALID_TENANT,
            skt=START, ske=EXPIRY, sks=key_service, skv=VERSION,
            skdutid=tenant, sduoid=OBJECT_ID,
            st=START, se=EXPIRY, sig="Sanitized", sv="Sanitized",
        )
        if service != "queue":
            query["sr"] = "f" if service == "file" else "b"
        else:
            query["comp"] = "metadata"
        uri = url(resource_service, path, **query)
        method = "GET" if service in ["blob", "queue"] else "HEAD"
        if tenant == TENANT:
            entries.append(entry(
                method, uri, body="synthetic" if service == "blob" else "",
                headers=({"x-ms-approximate-messages-count": "0"}
                         if service == "queue" else properties()),
                request_headers={"Authorization": "Sanitized"},
            ))
        else:
            entries.append(error(method, uri, "AuthorizationPermissionMismatch"))
    return entries


def bearer_challenge(service):
    uri = url(service, "container/blob") if service == "blob" else url(
        service, restype="service", comp="properties")
    method = "HEAD" if service == "blob" else "GET"
    challenge = (
        f'Bearer authorization_uri="https://login.microsoftonline.com/{TENANT}/oauth2/authorize" '
        'resource_id="https://storage.azure.com"'
    )
    service_properties = (
        "<StorageServiceProperties><Logging><Version>1.0</Version>"
        "<Delete>false</Delete><Read>false</Read><Write>false</Write>"
        "<RetentionPolicy><Enabled>false</Enabled></RetentionPolicy>"
        "</Logging><Cors /></StorageServiceProperties>"
    )
    entries = []
    for scenario in range(5):
        entries.append(entry(method, uri, 401, headers={"WWW-Authenticate": challenge}))
        if scenario == 4:
            break
        entries.append(token())
        entries.append(entry(
            method, uri, body=service_properties if service == "queue" else "",
            headers=properties() if service == "blob" else {"Content-Type": "application/xml"},
            request_headers={"Authorization": "Sanitized"},
        ))
        if service == "blob" and scenario == 0:
            entries.append(entry(
                "GET", uri, body="synthetic", headers=properties(),
                request_headers={"Authorization": "Sanitized"},
            ))
    entries.insert(-1, entry(
        "POST", "https://Sanitized.microsoftonline.com/oauth2/v2.0/token", 400,
        body=json.dumps({"error": "invalid_request", "error_description": "Synthetic missing tenant."}),
        headers={"Content-Type": "application/json"},
    ))
    return entries


def crc64(content):
    value = (1 << 64) - 1
    for byte in content:
        value ^= byte
        for _ in range(8):
            value = (value >> 1) ^ (0x9A6C9329AC4BC9B5 if value & 1 else 0)
    value ^= (1 << 64) - 1
    return base64.b64encode(value.to_bytes(8, "little")).decode()


def crc_condition(page_blob):
    uri = url("blob", "container/destination", comp="page" if page_blob else "appendblock")
    good_crc = crc64(b"a" * (4096 if page_blob else 10))
    bad = {"x-ms-source-content-crc64": "+DNR5PON4EM="}
    good = {"x-ms-source-content-crc64": good_crc}
    for request_headers in [bad, good]:
        request_headers["x-ms-copy-source"] = "Sanitized"
    if page_blob:
        for headers in [bad, good]:
            headers.update({
                "x-ms-source-range": "bytes=0-4095", "x-ms-range": "bytes=0-4095",
                "x-ms-page-write": "update",
            })
    headers = properties("PageBlob" if page_blob else "AppendBlob")
    headers.update({
        "Content-Length": "0",
        "x-ms-blob-sequence-number": "0",
        "x-ms-blob-append-offset": "0",
        "x-ms-blob-committed-block-count": "1",
        "x-ms-request-server-encrypted": "true",
    })
    return [
        error("PUT", uri, "Crc64Mismatch", 400, bad),
        entry("PUT", uri, 201, request_headers=good, headers=headers),
    ]


def rename_container():
    source = url("blob", "synthetic-src", restype="container")
    first = url("blob", "synthetic-dest1", restype="container")
    second = url("blob", "synthetic-dest2", restype="container")
    rename_first = first + "&comp=rename"
    rename_second = second + "&comp=rename"
    return [
        entry("PUT", source, 201, headers=properties()),
        entry("PUT", rename_first,
              request_headers={"x-ms-source-container-name": "synthetic-src"}),
        error("GET", source, "ContainerNotFound", 404),
        entry("GET", first, headers=properties()),
        entry("PUT", first + "&comp=lease", 201,
              request_headers={"x-ms-lease-action": "acquire", "x-ms-proposed-lease-id": LEASE,
                               "x-ms-lease-duration": "60"},
              headers={**properties(), "x-ms-lease-id": LEASE}),
        error("PUT", rename_second, "LeaseIdMismatchWithContainerOperation", 412,
              {"x-ms-source-container-name": "synthetic-dest1", "x-ms-source-lease-id": WRONG_LEASE}),
        entry("PUT", rename_second,
              request_headers={"x-ms-source-container-name": "synthetic-dest1",
                               "x-ms-source-lease-id": LEASE}),
        entry("GET", second, headers={**properties(), "ETag": '"synthetic-renamed-etag"'}),
        entry("DELETE", second, 202),
    ]


def pagination_delete():
    entries = []
    for continuation, following in [("", "page-2"), ("page-2", "page-3"), ("page-3", "")]:
        query = {"recursive": "true", "paginated": "true"}
        if continuation:
            query["continuation"] = continuation
        entries.append(entry(
            "DELETE", url("dfs", "filesystem/directory", **query), 200,
            headers={"x-ms-continuation": following} if following else {},
        ))
    return entries


def generate(assets_repo):
    fixtures = {
        "azure-storage-blobs": {
            "AppendBlockFromUriCrc64AccessCondition": crc_condition(False),
            "UploadPagesFromUriCrc64AccessCondition": crc_condition(True),
            "RenameBlobContainer": rename_container(),
            "BlobBearerChallengeWorks": bearer_challenge("blob"),
            "BlobPrincipalBoundDelegationSas_CrossTenant": cross_tenant("blob"),
        },
        "azure-storage-files-datalake": {
            "PaginationDelete": pagination_delete(),
            "DataLakePrincipalBoundDelegationSas_CrossTenant": cross_tenant("datalake"),
        },
        "azure-storage-files-shares": {
            "SharePrincipalBoundDelegationSas_CrossTenant": cross_tenant("file"),
        },
        "azure-storage-queues": {
            "QueueBearerChallengeWorks": bearer_challenge("queue"),
            "QueuePrincipalBoundDelegationSas_CrossTenant": cross_tenant("queue"),
        },
    }
    for package, tests in fixtures.items():
        directory = assets_repo / "cpp" / "sdk" / "storage" / package / "test" / "ut" / "recordings"
        for name, entries in tests.items():
            path = directory / f"SyntheticStorageTest.{name}_PLAYBACKONLY_.json"
            if path.exists():
                previous = json.loads(path.read_text(encoding="utf-8-sig"))
                if any(e["ResponseHeaders"].get("Server") != SERVER for e in previous["Entries"]):
                    raise ValueError(f"Refusing to overwrite a non-synthetic recording: {path}")
            directory.mkdir(parents=True, exist_ok=True)
            path.write_text(json.dumps({"Entries": entries, "Variables": {}}, indent=2) + "\n",
                            encoding="utf-8")
            print(f"{path.name}: {len(entries)} synthetic responses")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assets-repo", type=Path, required=True)
    generate(parser.parse_args().assets_repo.resolve())
