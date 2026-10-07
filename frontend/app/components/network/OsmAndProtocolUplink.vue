<script setup lang="ts">
import * as z from "zod";
import type { FormSubmitEvent } from "@nuxt/ui";

const schema = z.object({
  host: z.string(),
  port: z.number(),
  deviceId: z.string(),
  interval: z.string(),
});
type Schema = z.output<typeof schema>;

const state = reactive<Partial<Schema>>({
  host: "demo.traccar.org",
  port: 443,
  deviceId: "GPS-8842",
  interval: "30s",
});

async function onSubmit(event: FormSubmitEvent<Schema>) {
  console.log(event.data);
}

const items = ["5s", "10s", "30s", "60s"];
</script>

<template>
  <UCard :ui="{ body: 'space-y-4' }">
    <div class="flex items-center justify-between gap-2">
      <div class="flex items-center gap-2">
        <UBadge
          icon="i-lucide-satellite-dish"
          color="secondary"
          variant="soft"
          size="xl"
        />
        <div>
          <h2 class="font-medium text-xl">OsmAnd Protocol Uplink</h2>
          <h6 class="text-muted text-sm">
            Continous telemetry stream to tracking server
          </h6>
        </div>
      </div>
      <UBadge label="HTTP GET" variant="soft" :ui="{ label: 'text-default' }" />
    </div>
    <UForm :schema="schema" :state="state" class="space-y-4" @submit="onSubmit">
      <div class="flex gap-2">
        <UFormField class="shrink" label="Server Host" name="host">
          <UInput trailing-icon="i-lucide-hash" v-model="state.host" />
        </UFormField>

        <UFormField class="flex-1" label="Port" name="port">
          <UInput v-model="state.port" />
        </UFormField>
      </div>

      <UFormField label="Device Identifier (ID)" name="deviceId">
        <UInput class="w-full" v-model="state.deviceId" />
      </UFormField>

      <UFormField label="Reporting Interval" name="interval">
        <URadioGroup
          :ui="{ item: 'w-full' }"
          indicator="hidden"
          orientation="horizontal"
          variant="table"
          default-value="30s"
          :items="items"
        />
      </UFormField>
    </UForm>
    <UBadge class="flex items-center" color="secondary" variant="soft">
      <AnimatedPing class="flex-none" animate="pulse" />
      <div>
        <span class="text-muted text-xs">Last Ping: 12s ago • 200 OK</span>
        <br />
        <span class="text-muted text-xs">
          Latency: 40ms • payload: 12 B
        </span>
      </div>
      <UButton color="secondary" icon="i-lucide-send" label="Test Ping"/>
    </UBadge>
  </UCard>
</template>
