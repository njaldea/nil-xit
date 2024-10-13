<script>
    import { xit } from "@nil-/xit";

    /** @type import('@nil-/xit').FrameInfo[] */
    let frames = [
        {frame: "base"},
        {frame: "json_editor"},
        {frame: "tagged", tag: "1101"},
        {frame: "tagged", tag: "1102"},
        {frame: "group"},
    ];

    const { loader } = xit();

    let selected = $state(0);
</script>

<select bind:value={selected}>
    {#each frames as frame, i}
        <option value={i}>{frame.tag ? `${frame.frame}-${frame.tag}` : frame.frame}</option>
    {/each}
</select>

{#key selected}
    {#if selected < frames.length}
        {@const f = frames[selected]}
        {@const a = loader.one(f.frame, f.tag)}
        <div use:a></div>
    {/if}
{/key}
