<template>
  <UCard variant="soft" class="shadow-none">
    <div class="flex items-center gap-2">
      <AnimatedPing class="flex-none" />
      <h2 class="font-semibold text-xl">Hostpot & OsmAnd Uplink</h2>
      <UBadge
        label="AP Active"
        variant="soft"
        :ui="{ label: 'text-default' }"
      />
    </div>
    <div class="flex items-center gap-2 justify-between">
      <span class="inline-flex items-center gap-1 text-sm">
        <UIcon name="i-lucide-router" class="size-4" />
        192.168.4.1</span
      >
      <span class="inline-flex items-center gap-1 text-sm">
        <UIcon name="i-lucide-antenna" class="size-4" />
        Channel 6 • 2.4 GHz</span
      >
      <span class="inline-flex items-center gap-1 text-sm">
        <UIcon name="i-lucide-refresh-ccw" class="size-4" />
        Traccar Ready</span
      >
    </div>
  </UCard>
</template>
