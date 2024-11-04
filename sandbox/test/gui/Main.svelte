<script>
    import { xit, json_string } from "@nil-/xit";

    const { values, loader } = xit();
    const scenes = values.json("scenes", { scenes: [""] }, json_string);
    const selected = values.number("selected", 0);
</script>

<svelte:head>
    <title>nil - xit</title>
</svelte:head>

<div class="root">
    <select bind:value={$selected}>
        {#each $scenes.scenes as id, i}
            <option value={i}>{id}</option>
        {/each}
    </select>

    {#key $selected}
        {#if 0 < $selected && $selected < $scenes.scenes.length}
            {@const tag = $scenes.scenes[$selected]}
            {#await Promise.all([
                loader.one("view_frame", tag),
                loader.one("slider_frame", tag),
                loader.one("editor_frame", tag)
            ])}
                <div>Loading...</div>
            {:then [view, slider, editor]}
                <div class="root-content">
                    <div class="view">
                        <div style="display: contents" use:view></div>
                    </div>
                    <div class="pane">
                        <div style="display: contents" use:slider></div>
                        <div style="display: contents" use:editor></div>
                    </div>
                </div>
            {:catch}
                <div>Error during loading...</div>
            {/await}
        {/if}
    {/key}
</div>

<style>
    .root {
        display: flex;
        flex-direction: column;
    }

    .root-content {
        position: relative;
    }

    .view {
        width: calc(100% - 200px);
    }

    .pane {
        position: absolute;
        left: calc(100% - 200px);
        right: 0;
        top: 0;
        bottom: 0;
    }
</style>