<script>
    import Container from "@nil-/xit/components/layouts/Container.svelte";
    import { xit } from "@nil-/xit";

    /** @type {{ frame: string; tag?: string }[]} */
    let frames = [
        {frame: "base"},
        {frame: "json_editor"},
        {frame: "tagged", tag: "1101"},
        {frame: "tagged", tag: "1102"},
        {frame: "group"},
    ];

    const { load_frame_ui } = xit();

    let selected = $state(0);
</script>

<Container>
<div class="root">
    <select bind:value={selected}>
        {#each frames as frame, i}
            <option value={i}>{frame.tag ? `${frame.frame}-${frame.tag}` : frame.frame}</option>
        {/each}
    </select>

    {#key selected}
        {#if selected < frames.length}
            {@const f = frames[selected]}
            {#await load_frame_ui(f.frame, f.tag)}
                <div>Loading {f.tag ? `${f.frame}-${f.tag}` : f.frame}...</div>
            {:then a}
                <div style="display: contents" use:a></div>
            {/await}
        {/if}
    {/key}

    <table>
        <thead><tr><td>key</td><td>value</td></tr></thead>
        <tbody><tr><td>hello</td><td>{{hello}}</td></tr></tbody>
    </table>
</div>
</Container>

<style>
    td {
        outline: 1px solid red;
    }
    .root {
        display: flex;
        flex-direction: column;
        height: 100%;
    }
</style>