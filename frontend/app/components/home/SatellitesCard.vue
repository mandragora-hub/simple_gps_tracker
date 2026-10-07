<template>
  <UCard variant="outline">
    <div class="flex items-center gap-2">
      <AnimatedPing class="flex-none" />
      <div class="flex-auto min-w-0">
        <div>
          <span class="font-bold text-lg">14 Satellites </span>
          <UIcon name="i-lucide-satellite" class="text-primary-800 size-5" />
        </div>
        <span
          class="text-muted text-sm font-mono"
          >Neo-mbn • Glossnass / GPS</span
        >
      </div>
      <div class="shrink flex gap-2">
        <UBadge variant="soft" class="w-full" size="lg"
          ><span class="whitespace-nowrap text-sm text-default font-mono"
            >3D Fix <br />(± 1.4m)</span
          ></UBadge
        >
        <UBadge
          color="secondary"
          variant="soft"
          class="text-sm font-mono"
          size="lg"
          ><span>TTFF 5.2s</span></UBadge
        >
      </div>
    </div>
  </UCard>
</template>
