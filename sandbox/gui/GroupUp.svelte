<script>
    import { getContext } from "svelte";
    import { json_string } from "@nil-/xit";

    /** @type import('./nil-xit').Xit */
    const { binding, loader } = getContext("nil.xit");

    /** @type import('./nil-xit').Writable<{ links: string[] }> */
    const links = binding.json("links", { links: [] }, json_string);
</script>

<div class="wrapper">
    {#if $links.links.length > 0}
        {@const frames = loader.all($links.links)}
        <div use:frames></div>
    {/if}
    <!-- {#each $links.links as link}
        {@const frame = loader.one(link)}
        <div use:frame/>
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