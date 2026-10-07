<template>
  <div class="flex flex-col gap-4 mx-auto py-5">
    <SystemUptime />
    <SystemMCUResumeCard />
    <div class="flex items-center gap-4">
      <SystemUSBCard class="flex-auto" />
      <SystemInternalTempCard class="flex-auto" />
    </div>
    <!-- <SystemRawNMEAStreamCard /> -->
    <SystemOTACard />
  </div>
</template>
