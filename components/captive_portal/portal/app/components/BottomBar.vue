<script setup lang="ts">
import type { TabsItem } from "@nuxt/ui";

const model = defineModel<string | number>();
const props = defineProps<{
  items: TabsItem[];
}>();
</script>

<template>
  <UTabs
    v-model="model"
    :items="props.items"
    :content="false"
    :ui="{
      list: 'justify-around w-full',
      trigger: 'grow flex-col gap-1 py-1',
      label: 'text-[10px]/3',
    }"
    class="w-full"
  />
</template>
