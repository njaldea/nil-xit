<script>
    /** @type {{ label: string, placeholder: string, value: string }}  */
    let {
        value = $bindable(),
        label,
        placeholder,
    } = $props();
</script>

<div>
    <span>{label}</span>
    <input bind:value={value} placeholder={placeholder}/>
</div>