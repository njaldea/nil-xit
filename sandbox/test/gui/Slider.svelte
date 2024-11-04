<script>
    import { xit } from "@nil-/xit";
    const { values } = xit();
    const v = values.number("value", 0);
</script>

<input type="range" min={0} max={10} step={1} bind:value={$v}/>