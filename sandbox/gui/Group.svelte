<script>
    import { xit } from "@nil-/xit";

    const { load_frame_ui } = xit();

    /** @type {{ frame: string; tag?: string }[]} */
    const frames = [
        {frame: "base"},
        {frame: "json_editor"},
        {frame: "tagged", tag: "1101"},
        {frame: "tagged", tag: "1102"}
    ];
</script>

<div class="wrapper">
    {#each frames as ff}
        {#await load_frame_ui(ff.frame, ff.tag)}
            <div>Loading {ff.tag ? `${ff.frame}-${ff.tag}` : ff.frame}...</div>
        {:then f}
            <div style="display: contents" use:f></div>
        {/await}
    {/each}
</div>

<style>
    .wrapper {
        display: flex;
        flex-direction: column;
        height: 100%;
        width: 100%;
    }
</style>