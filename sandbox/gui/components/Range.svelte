<script>
    /** @type {{ min: number; max: number; value: number; step: number; label: string }}  */
    let {
        value = $bindable(),
        min,
        max,
        step,
        label
    } = $props();
</script>

<div>
    <span>{label}</span>
    <input type="range" {min} {max} {step} bind:value />
</div>
