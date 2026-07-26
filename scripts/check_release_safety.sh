#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

upstream_base="$(tr -d '[:space:]' < UPSTREAM_BASE_COMMIT)"
sdk_base="$(tr -d '[:space:]' < FREEINK_SDK_BASE_COMMIT)"
# The supplied CrossPoint 1.5 snapshot has no Git metadata, so its SD updater
# cannot be covered by UPSTREAM_BASE_COMMIT. Pin the exact reviewed integrated
# source (including the forced post-update splash); any later edit fails closed.
reviewed_sd_update_sha256="f5fef33aab37987c251da0727653ce17c5492397db6123622e7a797d0598e4b3"
# CrossPointTH safety hardening layered on the reviewed updater: the flasher
# validates again immediately before writing, reads every chunk back, and only
# then changes otadata. main.cpp calls the FreeInk recovery hatch before normal
# application initialization. Pin all three exact reviewed files so release
# builds fail closed if any of those guarantees changes.
crosspointth_flasher_cpp_sha256="5b6efe231225182788c0629b971b3435d3981162d7190011a90a169c3a136989"
crosspointth_flasher_h_sha256="d7fd52b2935f6f85cbf33336b86950d52d4e4b2306177617f89fd318e2646857"
crosspointth_main_sha256="12b52e7d3263f64e73ad229ca24ad0f348256685b8ea8a06e1b9a466a72d77ea"
crosspointth_restart_h_sha256="25f050c4838b7009c9e0a70947575e1bc4175674001b719d9bc7f1f0e8d310c1"
crosspointth_ota_activity_sha256="750f8ed99231db477eecebff5c1199aad9e7bbb784984a53cc43ff2bb59e8f3d"
crosspointth_state_h_sha256="cee8d496e150f1a1c0a5d928449fd0a8e65af38a1b9b21779354d0542e30941e"
crosspointth_state_cpp_sha256="f13240ad4efa2db9186059fbf2e55ad58870faa5578527b77abbd31b27eefb5e"

git cat-file -e "${upstream_base}^{commit}"

sensitive_paths=(
  partitions.csv
  src/platform
  lib/hal
  freeink-sdk/libs/display
  freeink-sdk/libs/hardware/BoardConfig
  freeink-sdk/libs/hardware/PowerManager
  src/network/OtaBootSwitch.cpp
  src/network/OtaBootSwitch.h
  src/network/OtaUpdater.cpp
  src/network/OtaUpdater.h
  src/activities/settings/SdFirmwareUpdateActivity.h
)

if ! git diff --quiet "$upstream_base" -- "${sensitive_paths[@]}"; then
  echo "ERROR: hardware, partition, OTA, or flashing code differs from the recorded upstream base." >&2
  git diff --stat "$upstream_base" -- "${sensitive_paths[@]}" >&2
  exit 1
fi

check_sha256() {
  local path="$1"
  local expected="$2"
  local actual
  actual="$(shasum -a 256 "$path" | awk '{print $1}')"
  if [[ "$actual" != "$expected" ]]; then
    echo "ERROR: $path differs from the reviewed CrossPointTH safety source." >&2
    echo "Expected: $expected" >&2
    echo "Actual:   $actual" >&2
    exit 1
  fi
}

check_sha256 src/network/FirmwareFlasher.cpp "$crosspointth_flasher_cpp_sha256"
check_sha256 src/network/FirmwareFlasher.h "$crosspointth_flasher_h_sha256"
check_sha256 src/main.cpp "$crosspointth_main_sha256"
check_sha256 src/SilentRestart.h "$crosspointth_restart_h_sha256"
check_sha256 src/activities/settings/OtaUpdateActivity.cpp "$crosspointth_ota_activity_sha256"
check_sha256 src/CrossPointState.h "$crosspointth_state_h_sha256"
check_sha256 src/CrossPointState.cpp "$crosspointth_state_cpp_sha256"

actual_sd_update_sha256="$(shasum -a 256 src/activities/settings/SdFirmwareUpdateActivity.cpp | awk '{print $1}')"
if [[ "$actual_sd_update_sha256" != "$reviewed_sd_update_sha256" ]]; then
  echo "ERROR: SD firmware updater differs from the reviewed CrossPointTH safety source." >&2
  echo "Expected: $reviewed_sd_update_sha256" >&2
  echo "Actual:   $actual_sd_update_sha256" >&2
  exit 1
fi

actual_sdk="$(git -C freeink-sdk rev-parse HEAD)"
if [[ "$actual_sdk" != "$sdk_base" ]]; then
  echo "ERROR: FreeInk SDK is $actual_sdk; expected $sdk_base." >&2
  exit 1
fi

echo "Release safety gate passed."
echo "Upstream base: $upstream_base"
echo "1.5 SD updater: $actual_sd_update_sha256"
echo "TH safe flasher: $crosspointth_flasher_cpp_sha256"
echo "TH recovery app: $crosspointth_main_sha256"
echo "FreeInk SDK:   $actual_sdk"
