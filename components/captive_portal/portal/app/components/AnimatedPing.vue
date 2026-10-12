<script setup lang="ts">
import type { Color, Size } from "~/types";

const props = withDefaults(
  defineProps<{ success?: boolean; animate?: "ping" | "pulse"; size?: Size }>(),
  {
    success: true,
    animate: "ping",
    size: "md",
  },
);

const sizes: Record<Size, string> = {
  xs: "size-1",
  sm: "size-2",
  md: "size-3",
  lg: "size-4",
  xl: "size-5",
  "2xl": "size-6",
};
</script>

<template>
  <span class="relative flex" :class="sizes[props.size]">
    <span
      class="absolute inline-flex h-full w-full rounded-full opacity-75"
      :class="{
        'bg-primary-600': props.success,
        'bg-neutral-600': !props.success,
        'animate-ping': props.animate == 'ping',
      }"
    ></span>
    <span
      class="relative inline-flex rounded-full"
      :class="[
        {
          'bg-primary-800': props.success,
          'bg-neutral-800': !props.success,
          'animate-pulse': props.animate == 'pulse',
        },
        sizes[props.size],
      ]"
    ></span>
  </span>
</template>
