<template>
  <UCard
    variant="outline"
    :ui="{ header: 'border-none flex items-center justify-between pb-0' }"
  >
    <template #header>
      <div class="inline-flex items-center gap-1">
        <UIcon name="i-ph-gps-fix" class="text-primary-800 size-6" />
        <span class="text-muted font-medium uppercase text-sm">
          Fix Coordinates
        </span>
      </div>
      <div class="flex items-center gap-2">
        <UButton icon="i-lucide-copy" size="md" color="neutral" variant="soft"
          >Copy</UButton
        >
        <UTooltip text="Open on google maps">
          <UButton
            to="https://google.maps"
            target="_blank"
            icon="i-lucide-external-link"
            size="md"
            variant="outline"
            color="secondary"
            tool
          />
        </UTooltip>
      </div>
    </template>
    <div class="flex flex-col gap-4">
      <div class="flex gap-2">
        <UBadge
          color="secondary"
          variant="soft"
          class="flex-auto flex flex-col items-start"
          size="lg"
          ><span class="text-sm text-muted">Latitude</span>
          <span class="text-xl text-default">32.4563° N</span>
        </UBadge>
        <UBadge
          color="secondary"
          variant="soft"
          class="flex-auto flex flex-col items-start"
          size="lg"
          ><span class="text-sm text-muted">Longitude</span>
          <span class="text-xl text-default">-122.4563° W</span>
        </UBadge>
      </div>
      <div class="flex gap-2">
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted">Altitude</span>
          <span class="text-sm text-default">32 m</span>
        </div>
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted">HDOP</span>
          <span class="text-sm text-default">0.87</span>
        </div>
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted">Epoch (UTC)</span>
          <span class="text-sm text-default">18:12:43</span>
        </div>
      </div>
    </div>
  </UCard>
</template>
