<script>
    import { xit, json_string } from "@nil-/xit";

    const { values, loader } = xit();
    /** @type import("@nil-/xit").Writable<string[]> */
    const tags = values.json("tags", [], json_string);
    /** @type import("@nil-/xit").Writable<string[]> */
    const views = values.json("view", [], json_string);
    /** @type import("@nil-/xit").Writable<string[]> */
    const pane = values.json("pane", [], json_string);
    let selected = 0;
</script>

<svelte:head>
    <title>nil - xit</title>
</svelte:head>

<div class="root">
    <select bind:value={selected}>
        {#each $tags as id, i}
            <option value={i}>{id}</option>
        {/each}
    </select>

    {#key selected}
        {#if 0 < selected && selected < $tags.length}
            {@const tag = $tags[selected]}
            {@const v_actions = Promise.all($views.map(v => loader.one(v, tag)))}
            {@const p_actions = Promise.all($pane.map(v => loader.one(v, tag)))}
            {#await Promise.all([ v_actions, p_actions ])}
                <div>Loading...</div>
            {:then [ view_actions, pane_actions ]}
                <div class="root-content">
                    <div class="view">
                        {#await view_actions then a}
                            {#each a as action}
                                <div style="display: contents" use:action></div>
                            {/each}
                        {/await}
                    </div>
                    <div class="pane">
                        {#await pane_actions then a}
                            {#each a as action}
                                <div style="display: contents" use:action></div>
                            {/each}
                        {/await}
                    </div>
                </div>
            {:catch}
                <div>Error during loading...</div>
            {/await}
        {:else}
            <div>Nothing to load...</div>
        {/if}
    {/key}
</div>

<style>
    .root {
        height: 100%;
        display: flex;
        flex-direction: column;
    }

    .root-content {
        position: relative;
        height: 100%;
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