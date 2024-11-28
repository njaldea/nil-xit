<script>
    import { xit } from "@nil-/xit";

    /** @type {{ frame: string; tag?: string }[]} */
    let frames = [
        {frame: "base"},
        {frame: "json_editor"},
        {frame: "tagged", tag: "1101"},
        {frame: "tagged", tag: "1102"},
        {frame: "group"},
    ];

    const { frame_ui } = xit();

    let selected = $state(0);
</script>

<div class="root">
    <select bind:value={selected}>
        {#each frames as frame, i}
            <option value={i}>{frame.tag ? `${frame.frame}-${frame.tag}` : frame.frame}</option>
        {/each}
    </select>

    {#key selected}
        {#if selected < frames.length}
            {@const f = frames[selected]}
            {#await frame_ui(f.frame, f.tag)}
                <div>Loading {f.tag ? `${f.frame}-${f.tag}` : f.frame}...</div>
            {:then a}
                <div style="display: contents" use:a></div>
            {/await}
        {/if}
    {/key}
</div>

<style>
    .root {
        display: flex;
        flex-direction: column;
        height: 100%;
    }
</style>