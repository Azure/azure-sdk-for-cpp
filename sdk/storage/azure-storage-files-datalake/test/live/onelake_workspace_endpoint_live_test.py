# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

import json
import pathlib
import subprocess
import sys
import tempfile


def run_harness(binary, fixture, output, mode, expected_success):
    command = [
        str(binary),
        "--validate-only" if mode == "validate" else "--mock",
        "--manifest",
        str(fixture),
        "--require-cases=2",
        "--output",
        str(output),
    ]
    result = subprocess.run(command, check=False, capture_output=True, text=True)
    if (result.returncode == 0) != expected_success:
        raise RuntimeError(
            f"unexpected exit {result.returncode} for {fixture.name}:\n"
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )
    with output.open(encoding="utf-8") as stream:
        evidence = json.load(stream)
    if evidence["mode"] != mode:
        raise RuntimeError(f"unexpected mode for {fixture.name}: {evidence['mode']}")
    return evidence, result


def main():
    binary = pathlib.Path(sys.argv[1])
    fixtures = pathlib.Path(sys.argv[2])
    expected_summary = {
        "required": 2,
        "executed": 2,
        "passed": 2,
        "failed": 0,
        "skipped": 0,
    }

    with tempfile.TemporaryDirectory() as temporary_directory:
        temporary_path = pathlib.Path(temporary_directory)
        valid_fixture = fixtures / "valid-two-row.tsv"
        for mode in ("validate", "mock"):
            evidence, _ = run_harness(
                binary,
                valid_fixture,
                temporary_path / f"{mode}.json",
                mode,
                True,
            )
            if evidence["summary"] != expected_summary:
                raise RuntimeError(f"unexpected {mode} summary: {evidence['summary']}")
            expected_probe_attempts = 0 if mode == "validate" else 2
            if evidence["probeAttempts"] != expected_probe_attempts:
                raise RuntimeError(
                    f"unexpected {mode} probe count: {evidence['probeAttempts']}"
                )
            expected_result_keys = {
                "line",
                "cloud",
                "ring",
                "apiFamily",
                "status",
                "message",
            }
            if any(set(result) != expected_result_keys for result in evidence["results"]):
                raise RuntimeError(f"raw manifest fields leaked in {mode} evidence")

        skipped_fixture = temporary_path / "skipped.tsv"
        skipped_lines = valid_fixture.read_text(encoding="utf-8").splitlines()
        skipped_fields = skipped_lines[1].split("\t")
        skipped_fields[-1] = ""
        skipped_lines[1] = "\t".join(skipped_fields)
        skipped_fixture.write_text("\n".join(skipped_lines) + "\n", encoding="utf-8")

        for fixture_name in ("malformed", "duplicate", "missing", "skipped", "failed"):
            fixture = (
                skipped_fixture
                if fixture_name == "skipped"
                else fixtures / f"{fixture_name}.tsv"
            )
            evidence, _ = run_harness(
                binary,
                fixture,
                temporary_path / f"{fixture_name}.json",
                "validate",
                False,
            )
            expected_negative_summaries = {
                "malformed": {"required": 2, "executed": 2, "passed": 1, "failed": 1, "skipped": 0},
                "duplicate": {"required": 2, "executed": 2, "passed": 1, "failed": 1, "skipped": 0},
                "missing": {"required": 2, "executed": 1, "passed": 1, "failed": 0, "skipped": 0},
                "skipped": {"required": 2, "executed": 2, "passed": 1, "failed": 0, "skipped": 1},
                "failed": {"required": 2, "executed": 2, "passed": 1, "failed": 1, "skipped": 0},
            }
            if evidence["summary"] != expected_negative_summaries[fixture_name]:
                raise RuntimeError(
                    f"unexpected summary for {fixture_name}: {evidence['summary']}"
                )
            if evidence["probeAttempts"] != 0:
                raise RuntimeError(f"negative fixture was probed: {fixture_name}")

        control_fixture = temporary_path / "control.tsv"
        control_fixture.write_text(
            valid_fixture.read_text(encoding="utf-8").replace(
                "item-id/Files/probe.txt", "item-id/Files/probe\x0b.txt"
            ),
            encoding="utf-8",
        )
        control_evidence, _ = run_harness(
            binary,
            control_fixture,
            temporary_path / "control.json",
            "validate",
            True,
        )
        if "probePath" in control_evidence["results"][1]:
            raise RuntimeError("control-character path leaked through JSON evidence")

        unicode_fixture = temporary_path / "unicode.tsv"
        unicode_probe_path = "item-id/Files/caf\u00e9.txt"
        unicode_fixture.write_text(
            valid_fixture.read_text(encoding="utf-8").replace(
                "item-id/Files/probe.txt", unicode_probe_path
            ),
            encoding="utf-8",
        )
        unicode_evidence, _ = run_harness(
            binary,
            unicode_fixture,
            temporary_path / "unicode.json",
            "validate",
            True,
        )
        if unicode_probe_path in json.dumps(unicode_evidence, ensure_ascii=False):
            raise RuntimeError("Unicode probe path leaked through JSON evidence")

        malformed_utf8_fixture = temporary_path / "malformed-utf8.tsv"
        malformed_utf8_fixture.write_bytes(
            valid_fixture.read_bytes().replace(
                b"item-id/Files/probe.txt", b"item-id/Files/probe-\xff.txt"
            )
        )
        for mode in ("validate", "mock"):
            malformed_evidence, _ = run_harness(
                binary,
                malformed_utf8_fixture,
                temporary_path / f"malformed-utf8-{mode}.json",
                mode,
                False,
            )
            expected_malformed_summary = {
                "required": 2,
                "executed": 2,
                "passed": 0,
                "failed": 2,
                "skipped": 0,
            }
            if malformed_evidence["summary"] != expected_malformed_summary:
                raise RuntimeError(
                    f"unexpected malformed UTF-8 summary: {malformed_evidence['summary']}"
                )
            if malformed_evidence["probeAttempts"] != 0:
                raise RuntimeError("malformed UTF-8 reached the probe phase")

        secret = "TOPSECRET_SENTINEL"
        secret_fixture = temporary_path / "secret.tsv"
        secret_fixture.write_text(
            valid_fixture.read_text(encoding="utf-8").replace(
                "https://onelake.blob.fabric.microsoft.com",
                f"https://onelake.blob.fabric.microsoft.com?sig={secret}",
            ),
            encoding="utf-8",
        )
        secret_output = temporary_path / "secret.json"
        secret_evidence, secret_result = run_harness(
            binary,
            secret_fixture,
            secret_output,
            "mock",
            False,
        )
        disclosed_text = (
            secret_output.read_text(encoding="utf-8")
            + secret_result.stdout
            + secret_result.stderr
        )
        if secret in disclosed_text:
            raise RuntimeError("secret from rejected manifest URL was disclosed")
        if secret_evidence["probeAttempts"] != 0:
            raise RuntimeError("rejected secret-bearing URL reached the probe phase")

        full_device = pathlib.Path("/dev/full")
        if full_device.exists():
            result = subprocess.run(
                [
                    str(binary),
                    "--validate-only",
                    "--manifest",
                    str(valid_fixture),
                    "--require-cases=2",
                    "--output",
                    str(full_device),
                ],
                check=False,
                capture_output=True,
                text=True,
            )
            if result.returncode == 0:
                raise RuntimeError("writing evidence to /dev/full unexpectedly succeeded")


if __name__ == "__main__":
    main()