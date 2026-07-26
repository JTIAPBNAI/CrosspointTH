#pragma once

// ESP.restart() with an RTC_NOINIT flag that survives the reboot, so setup()
// skips the boot splash and routes straight to a destination. Used to clear
// heap fragmentation accumulated during a wifi session.

void silentRestart();          // home screen
void silentRestartToReader();  // currently-open EPUB (APP_STATE.openEpubPath)

// Plain restart that forces the next boot to render the splash, even if a
// persisted quick-resume flag would otherwise suppress it. Used after a
// successful firmware install so the newly installed version is visible.
void restartShowingSplash();
