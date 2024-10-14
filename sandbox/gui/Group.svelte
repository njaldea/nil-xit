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
    {#if frames.length > 0}
        {@const f = loader.all(frames)}
        <div use:f></div>
    {/if}
    <!-- {#each frames as ff}
        {@const f = loader.one(ff.frame, ff.tag)}
        <div use:f></div>
    {/each} -->
</div>

<style>
    .wrapper {
        display: flex;
        flex-direction: column;
        height: 100%;
        width: 100%;
    }
</style>