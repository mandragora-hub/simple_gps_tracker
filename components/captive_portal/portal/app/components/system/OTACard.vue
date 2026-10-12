<script setup lang="ts">
const value = ref(30);
const items = ["1 Hz", "5 Hz", "10 Hz"];
</script>

<template>
  <UCard :ui="{ body: 'space-y-4' }">
    <div class="flex items-center gap-2">
      <UIcon name="i-lucide-wrench" class="size-6" />
      <h2 class="font-medium text-xl">Maintenance & OTA</h2>
    </div>
    <UBadge
      variant="soft"
      color="secondary"
      class="flex flex-col items-start p-2 gap-2"
    >
      <div class="flex items-end justify-between w-full">
        <h2 class="font-sans text-xl text-default">Firmware Flash (OTA)</h2>
        <UBadge label="v2.4.0" color="secondary" variant="soft" />
      </div>
      <h6 class="font-sans text-md text-muted">
        Upload compiled ESP32 firmaware (.bin)
      </h6>
      <UButton
        label="Choose .bin file"
        icon="i-lucide-upload"
        size="xl"
        color="secondary"
        variant="soft"
        class="flex justify-center align-center w-full"
        :ui="{ label: 'text-right' }"
      />
      <div class="flex items-center gap-2 w-full">
        <UButton
          label="Restart ESP32"
          icon="i-lucide-refresh-ccw"
          color="secondary"
          variant="soft"
          class="flex-auto"
        />
        <UButton
          label="Format SD Card"
          icon="i-lucide-lab-card-sd"
          color="error"
          variant="soft"
          class="flex-auto"
        />
      </div>
    </UBadge>
  </UCard>
</template>
