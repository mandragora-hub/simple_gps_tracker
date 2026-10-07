<script setup lang="ts">
import type { ScreenItem } from "~/types";
const ScreenFallback = resolveComponent("ScreenFallback");

const active = defineModel();
const props = defineProps<{ items: ScreenItem[] }>();

const currentScreen = () => {
  const screen = props.items.find((el) => el.value == active.value)?.screen;
  return screen ? screen : ScreenFallback;
};
</script>

<template>
  <div>
    <component :is="currentScreen()" />
  </div>
</template>
