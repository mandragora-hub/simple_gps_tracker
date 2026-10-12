<template>
  <UCard
    variant="outline"
    :ui="{ header: 'border-none flex items-center justify-between pb-0' }"
  >
    <template #header>
      <div class="inline-flex items-center gap-1">
        <AnimatedPing animate="pulse" />
        <span class="font-medium uppercase text-sm"> Session Logging </span>
      </div>
      <UBadge color="secondary" variant="soft"
        ><span class="text-sm text-muted">CSV / GPX active</span>
      </UBadge>
    </template>
    <div class="flex flex-col gap-4">
      <UBadge color="secondary" variant="soft" class="flex gap-2" size="lg">
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted">Distance</span>
          <span class="text-lg text-default">12.4 km</span>
        </div>
        <USeparator orientation="vertical" class="p-2 h-10" />
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted">Duration</span>
          <span class="text-lg text-default">00:21:42</span>
        </div>
        <USeparator orientation="vertical" class="p-2 h-10" />
        <div class="flex-auto flex flex-col items-end">
          <span class="text-sm text-muted">Flash free</span>
          <span class="text-lg text-default">34 MB</span>
        </div>
      </UBadge>
      <div class="flex gap-2">
        <UButton
          label="Pause"
          icon="i-lucide-pause"
          color="secondary"
          variant="soft"
          class="flex-auto"
          size="xl"
        />
        <UButton
          label="Stop & Save"
          icon="i-lucide-save"
          variant="solid"
          class="flex-auto"
          size="xl"
        />
      </div>
    </div>
  </UCard>
</template>
