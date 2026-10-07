<script setup lang="ts">
const value = ref(30);
const items = ["1 Hz", "5 Hz", "10 Hz"];
</script>

<template>
  <UCard :ui="{ body: 'space-y-4' }">
    <div class="flex items-center justify-between gap-2">
      <div class="flex items-center gap-2">
        <UIcon name="i-lucide-cpu" class="size-6" />
        <h2 class="font-medium text-xl">Microcontroller & GNSS</h2>
      </div>
      <UBadge
        label="REV 3.1"
        variant="soft"
        color="secondary"
        :ui="{ label: 'text-default' }"
      />
    </div>
    <UBadge
      variant="soft"
      color="secondary"
      class="flex flex-col items-start p-2"
    >
      <span class="uppercase text-sm">Processor core</span>
      <h2 class="font-sans text-xl text-default">ESP32-WROOM-32D</h2>
      <span class="text-sm">240 MHz Dual Core Xtensa</span>
    </UBadge>
    <UBadge
      variant="soft"
      color="secondary"
      class="flex flex-col items-start p-2"
    >
      <span class="uppercase text-sm">Gnss Sensor</span>
      <h2 class="font-sans text-xl text-default">u-blox NEO-8M / M9N</h2>
      <span class="text-sm text-primary inline-flex items-center gap-2"
        ><AnimatedPing />3D Fix • 18 Sats Locked</span
      >
    </UBadge>
    <UBadge
      variant="soft"
      color="secondary"
      class="flex flex-col items-start p-2 gap-2"
    >
      <div class="flex items-center justify-between w-full">
        <span class="uppercase text-sm">Free heap memory</span>
        <span class="text-sm"
          ><bold class="text-highlighted">218 KB</bold> / 512 KB</span
        >
      </div>
      <UProgress v-model="value" />
      <div class="flex items-center justify-between w-full">
        <span class="text-sm">Fragment: 4.2%</span>
        <span class="text-sm">Allocated : 294 KB</span>
      </div>
    </UBadge>
    <div class="space-y-2">
      <div class="flex items-center justify-between w-full">
        <span class="text-sm">GNSS Update Rate</span>
        <span class="text-sm">100 ms Interval</span>
      </div>
      <URadioGroup
        :ui="{ item: 'w-full' }"
        indicator="hidden"
        orientation="horizontal"
        variant="table"
        default-value="5 Hz"
        :items="items"
      />
    </div>
  </UCard>
</template>
