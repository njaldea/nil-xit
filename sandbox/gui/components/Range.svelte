<script>
    /** @type {{ min: number; max: number; value: number; step: number; label: string }}  */
    let { min, max, value, step, label } = $props();
</script>

<div>
    <span>{label}</span>
    <input type="range" {min} {max} {step} bind:value />
</div>
