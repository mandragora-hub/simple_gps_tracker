<script setup lang="ts">
const value = ref(30);
</script>

<template>
  <UCard variant="outline" :ui="{ body: 'flex flex-col gap-2' }">
    <div class="flex items-center justify-between">
      <div class="inline-flex items-center gap-1">
        <UIcon name="i-lucide-lab-card-sd" class="size-4" />
        <span class="font-medium text-sm"> MicroSD Storage </span>
      </div>
      <span class="text-muted text-sm">1.2 GB of 4 GB used (8%)</span>
    </div>
    <UProgress v-model="value" />
    <div class="flex items-center justify-between">
      <span class="text-primary text-sm">Class 10 UHS-I • FAT32</span>
      <span class="text-primary text-sm">12.4 GB Free(~432 hrs)</span>
    </div>
  </UCard>
</template>
