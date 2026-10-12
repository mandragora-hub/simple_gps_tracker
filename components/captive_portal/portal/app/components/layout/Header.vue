<template>
  <header
    class="backdrop-blur-sm border-b border-default h-(--ui-header-height) sticky top-0 z-50"
  >
    <UContainer class="h-full flex items-center justify-between">
      <slot name="title">
        <!-- <Logo class="h-6 w-auto" /> -->
        <div>
          <div class="flex items-baseline gap-2">
            <h1 class="font-semibold text-2xl">SimpleGPS</h1>
            <UKbd color="secondary" variant="soft">ESP32-WROOM</UKbd>
          </div>
          <small class="text-xs text-toned">Configuration & monitoring tools</small>
        </div>
      </slot>
      <div>
        <slot name="right">
          <UColorModeButton />
          <ThemeButton />
        </slot>
      </div>
    </UContainer>
  </header>
</template>
