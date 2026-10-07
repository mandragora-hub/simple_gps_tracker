<script setup lang="ts">
import * as z from "zod";
import type { FormSubmitEvent } from "@nuxt/ui";

const value = ref(true);

const schema = z.object({
  autoSyncFeatureActive: z.boolean(),
  url: z.string(),
});
type Schema = z.output<typeof schema>;

const state = reactive<Partial<Schema>>({
  autoSyncFeatureActive: false,
  url: "http://nas.local:8080/api/gps/upload",
});

async function onSubmit(event: FormSubmitEvent<Schema>) {
  console.log(event.data);
}
</script>

<template>
  <UCard :ui="{ body: 'space-y-4' }">
    <div class="flex items-center justify-between gap-2">
      <div class="flex items-center gap-2">
        <div>
          <h2 class="font-medium text-xl">
            Auto-sync GPX to Home Server
            <UIcon name="i-lucide-cloud-upload" class="size-4" />
          </h2>
          <h6 class="text-muted text-sm">
            Pushes completed track logs when connected
          </h6>
        </div>
      </div>
      <USwitch v-model="value" />
    </div>
    <UForm :schema="schema" :state="state" class="space-y-4" @submit="onSubmit">
      <UFormField class="shrink" label="Webhook Endpoint URL" name="url">
        <UInput
          trailing-icon="i-lucide-link-2"
          class="w-full"
          v-model="state.url"
        />
      </UFormField>
    </UForm>
    <span class="text-muted text-sm font-sans">
      Target receives raw NMEA-0183 & GPX v1.1 files upon geofences exit.
    </span>
  </UCard>
</template>
