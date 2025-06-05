<script lang="ts">
    import { hello } from "./hello.ts";

    let {
        value = $bindable(),
        min,
        max,
        step,
        label
    }: { min: number; max: number; value: number; step: number; label: string } = $props();
</script>

<div>
    {hello}
    <span>{label}</span>
    <input type="range" {min} {max} {step} bind:value />
</div>
