<template>
  <UCard
    variant="outline"
    :ui="{ header: 'border-none flex items-center justify-between' }"
  >
    <template #header>
      <span class="text-default font-medium uppercase text-sm">Internal Temp </span>
      <UIcon name="i-lucide-thermometer" class="size-6" />
    </template>
    <div class="flex flex-col gap-2">
      <span class="text-2xl text-default">39.3 °C</span>
      <span class="text-sm text-highlighted inline-flex items-center gap-2"
        ><AnimatedPing size="sm" />Normal Range</span
      >
    </div>
  </UCard>
</template>
