<script setup lang="ts">
const trips = [1, 2, 3];
</script>

<template>
  <div class="flex items-center justify-between">
    <div class="inline-flex items-center gap-1">
      <span class="font-medium text-xl font-sans">Trip Logs</span>
      <UBadge label="3 files" color="secondary" variant="soft" />
    </div>
    <span class="inline-flex items-center gap-2 text-muted text-sm">
      <UIcon name="i-lucide-refresh-ccw" class="size-4" />
      Scan SD</span
    >
  </div>
  <div class="space-y-4 h-64 overflow-auto">
    <UCard
      v-for="trip in trips"
      variant="outline"
      :ui="{ body: 'flex flex-col gap-2' }"
    >
      <div class="flex items-center justify-between">
        <div class="inline-flex items-center gap-1">
          <UIcon name="i-lucide-map-pin" class="size-4" />
          <span class="font-medium text-md">Trip_2026-05-18_1420.gpx</span>
        </div>
        <span class="text-muted text-sm text-end">May 17, 08:15</span>
      </div>
      <div class="flex items-center justify-between">
        <div class="flex items-center gap-4">
          <span class="inline-flex items-center gap-1 text-sm">
            <UIcon name="i-lucide-ruler" class="size-4" />
            42.8 km</span
          >
          <span class="inline-flex items-center gap-1 text-sm">
            <UIcon name="i-lucide-watch" class="size-4" />
            42 min</span
          >
          <span class="inline-flex items-center gap-1 text-sm">
            <UIcon name="i-lucide-save" class="size-4" />
            4.2 MB</span
          >
        </div>
        <UButton
          label="GPX"
          icon="i-lucide-download"
          variant="soft"
          color="secondary"
        />
      </div>
    </UCard>
  </div>
  <UButton
    label="Download All Logs (.zip)"
    icon="i-lucide-hard-drive-download"
    size="xl"
    class="flex justify-center align-center"
    :ui="{ label: 'text-right' }"
  />
  <span class="inline-flex justify-center items-center gap-1 text-sm">
    <UIcon name="i-lucide-wifi" class="size-4" />
    High-speed direct transport over ESP32 AP</span
  >
</template>
