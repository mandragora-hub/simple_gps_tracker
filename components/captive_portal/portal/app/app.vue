<script setup lang="ts">
import type { ScreenItem } from "./types";

const ScreenHome = resolveComponent("ScreenHome");
const ScreenNetwork = resolveComponent("ScreenNetwork");
const ScreenSystem = resolveComponent("ScreenSystem");
const ScreenTrips = resolveComponent("ScreenTrips");

const activePage = ref("home");

const items: ScreenItem[] = [
  {
    label: "Home",
    value: "home",
    icon: "i-lucide-house",
    screen: ScreenHome,
  },
  {
    label: "Activity - trips",
    value: "activity",
    icon: "i-lucide-activity",
    screen: ScreenTrips,
  },
  {
    label: "Network",
    value: "network",
    icon: "i-lucide-network",
    screen: ScreenNetwork,
  },
  {
    label: "System",
    value: "System",
    icon: "i-lucide-terminal",
    screen: ScreenSystem,
  },
];
</script>

<template>
  <UApp>
    <LayoutHeader class="bg-elevated"/>
    <UMain class="flex flex-col bg-elevated">
      <UContainer class="flex-auto">
        <ScreenMap v-model="activePage" :items="items" />
      </UContainer>
      <BottomBar v-model="activePage" :items="items" class="flex-none shadow sticky bottom-0 z-50" />
    </UMain>
    <!-- <USeparator icon="i-simple-icons-nuxtdotjs" type="dashed" class="h-px" />

    <UFooter>
      <template #left>
        <p class="text-muted text-sm">
          Copyright © {{ new Date().getFullYear() }}
        </p>
      </template>
    </UFooter> -->
  </UApp>
</template>
