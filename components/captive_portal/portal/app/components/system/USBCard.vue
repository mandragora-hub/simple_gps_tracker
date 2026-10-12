<template>
  <UCard
    variant="outline"
    :ui="{ header: 'border-none flex items-center justify-between' }"
  >
    <template #header>
      <span class="text-default font-medium uppercase text-sm">USB VBUS </span>
      <UIcon name="i-lucide-zap" class="size-6" />
    </template>
    <div class="flex flex-col gap-2">
      <span class="text-2xl text-default">5.05 V</span>
      <span class="text-sm text-highlighted inline-flex items-center gap-2"
        ><AnimatedPing size="sm" />Good (Regulated)</span
      >
    </div>
  </UCard>
</template>
