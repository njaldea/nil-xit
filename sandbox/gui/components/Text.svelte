<script lang="ts">
    let {
        value = $bindable(),
        label,
        placeholder,
    }: { label: string, placeholder: string, value: string } = $props();
</script>

<div>
    <span>{label}</span>
    <input bind:value={value} placeholder={placeholder}/>
</div>