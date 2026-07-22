#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

upstream_base="$(tr -d '[:space:]' < UPSTREAM_BASE_COMMIT)"
sdk_base="$(tr -d '[:space:]' < FREEINK_SDK_BASE_COMMIT)"

git cat-file -e "${upstream_base}^{commit}"

sensitive_paths=(
  partitions.csv
  src/platform
  lib/hal
  freeink-sdk/libs/display
  freeink-sdk/libs/hardware/BoardConfig
  freeink-sdk/libs/hardware/PowerManager
  src/network/FirmwareFlasher.cpp
  src/network/FirmwareFlasher.h
  src/network/OtaBootSwitch.cpp
  src/network/OtaBootSwitch.h
  src/network/OtaUpdater.cpp
  src/network/OtaUpdater.h
  src/activities/settings/SdFirmwareUpdateActivity.cpp
  src/activities/settings/SdFirmwareUpdateActivity.h
)

if ! git diff --quiet "$upstream_base" -- "${sensitive_paths[@]}"; then
  echo "ERROR: hardware, partition, OTA, or flashing code differs from the recorded upstream base." >&2
  git diff --stat "$upstream_base" -- "${sensitive_paths[@]}" >&2
  exit 1
fi

actual_sdk="$(git -C freeink-sdk rev-parse HEAD)"
if [[ "$actual_sdk" != "$sdk_base" ]]; then
  echo "ERROR: FreeInk SDK is $actual_sdk; expected $sdk_base." >&2
  exit 1
fi

echo "Release safety gate passed."
echo "Upstream base: $upstream_base"
echo "FreeInk SDK:   $actual_sdk"
