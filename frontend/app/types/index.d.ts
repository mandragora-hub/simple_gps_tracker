import type { TabsItem } from "@nuxt/ui";
import type {
  Component,
  ConcreteComponent,
  ComputedOptions,
  MethodOptions,
} from "vue";

export interface ScreenItem extends TabsItem {
  screen?:
    | Component
    | string
    | ConcreteComponent<{}, any, any, ComputedOptions, MethodOptions, {}, any>
    | string;
}

export type Color =
  | "error"
  | "primary"
  | "secondary"
  | "success"
  | "info"
  | "warning"
  | "neutral";

export type Size = "xs" | "sm" | "md" | "lg" | "xl" | "2xl";
