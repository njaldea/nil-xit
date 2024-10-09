<script>
    import { json_string } from "@nil-/xit";
    import { getContext } from "svelte";

    /** @type import('@nil-/xit').Xit */
    const { binding, listeners } = getContext("nil.xit");

    const int_binding = binding.number('tagged_bind', 1101);

    const click = () => {
        int_binding.update(v => v + 1);
    }
</script>

<button onclick={click}>tagged {$int_binding}</button>