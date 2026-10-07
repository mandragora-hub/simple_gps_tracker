<template>
  <UCard
    variant="outline"
    :ui="{ header: 'border-none flex items-center justify-between pb-0' }"
  >
    <template #header>
      <div class="inline-flex items-center gap-1">
        <AnimatedPing class="flex-none" />
        <span class="text-primary font-medium uppercase text-sm">
          Active session
        </span>
      </div>
      <UBadge
        color="secondary"
        variant="soft"
        class="flex-none"
        size="lg"
        label="10 Hz logging"
      />
    </template>
    <div class="flex flex-col gap-4">
      <div class="flex gap-2">
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted uppercase">Duration</span>
          <span class="text-2xl text-default">00:23:11</span>
        </div>
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted uppercase">Distance</span>
          <span class="text-2xl text-default"
            >18 <span class="text-sm text-muted">km</span></span
          >
        </div>
        <div class="flex-auto flex flex-col items-start">
          <span class="text-sm text-muted uppercase">AVG Speed</span>
          <span class="text-2xl text-default"
            >23 <span class="text-sm text-muted">km/h</span></span
          >
        </div>
      </div>
      <UBadge color="secondary" variant="soft" class="flex-none flex justify-between" size="lg">
        <div class="inline-flex items-center gap-2">
          <div class="outline-2 outline-solid rounded-full w-4 h-4"></div>
          <span>Session_0314.log</span>
        </div>
        <UButton
          label="Finish & Save"
          icon="i-lucide-disc"
          color="error"
          variant="ghost"
          class="flex-none"
        />
      </UBadge>
    </div>
  </UCard>
</template>
