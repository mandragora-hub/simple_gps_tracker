<script setup lang="ts">
import * as z from "zod";
import type { FormSubmitEvent } from "@nuxt/ui";

const schema = z.object({ ssid: z.string(), password: z.string() });
type Schema = z.output<typeof schema>;

const state = reactive<Partial<Schema>>({
  ssid: "SimpleGsp-AP",
  password: undefined,
});

async function onSubmit(event: FormSubmitEvent<Schema>) {
  console.log(event.data);
}
</script>

<template>
  <UCard :ui="{ body: 'space-y-4' }">
    <div class="flex items-center justify-between gap-2">
      <div class="flex items-center gap-2">
        <UBadge
          icon="i-lucide-signal"
          color="secondary"
          variant="soft"
          size="xl"
        />
        <div>
          <h2 class="font-medium text-xl">Devices Hostpot (SoftAP)</h2>
          <h6 class="text-muted text-xs">
            Captive configuration portal interface
          </h6>
        </div>
      </div>
      <UBadge
        label="wlan0"
        color="secondary"
        variant="soft"
        :ui="{ label: 'text-default' }"
      />
    </div>
    <UForm :schema="schema" :state="state" class="space-y-4" @submit="onSubmit">
      <UFormField label="Network Name (SSID)" name="ssid">
        <UInput class="w-full" trailing-icon="i-lucide-hash" v-model="state.ssid" />
      </UFormField>

      <UFormField label="Wi-Fi Password (WPA2)" name="password">
        <UInput class="w-full" v-model="state.password" type="password" />
      </UFormField>
    </UForm>
  </UCard>
</template>
