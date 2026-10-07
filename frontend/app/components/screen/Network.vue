<template>
  <div class="flex flex-col gap-4 mx-auto py-5">
    <NetworkResumeCard />
    <NetworkDeviceHostport />
    <NetworkOsmAndProtocolUplink />
    <NetworkAutoSyncGPX />
    <UButton
      label="Save Network Settings"
      icon="i-lucide-save"
      size="xl"
      class="flex justify-center align-center"
      :ui="{ label: 'text-right' }"
    />
    <span class="text-sm inline-flex items-center gap-2"
      ><UIcon name="i-lucide-cpu" class="size-6" />Flash EEPROM (Sector 0x1A) •
      Writes commit immediately</span
    >
  </div>
</template>
