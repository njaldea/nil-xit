<script>
    import { xit } from "@nil-/xit";

    const { loader } = xit();

    /** @type import('@nil-/xit').FrameInfo[] */
    const frames = [
        {frame: "base"},
        {frame: "json_editor"},
        {frame: "tagged", tag: "1101"},
        {frame: "tagged", tag: "1102"}
    ];
</script>

<div class="wrapper">
    <!-- {#if frames.length > 0}
        {#await loader.all(frames)}
            <div>Loading...</div>
        {:then f}
            <div style="display: contents" use:f></div>
        {/await}
    {/if} -->
    {#each frames as ff}
        {#await loader.one(ff.frame, ff.tag)}
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