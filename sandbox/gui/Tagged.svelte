<script>
    import { json_string } from "@nil-/xit";
    import { getContext } from "svelte";

    /** @type import('@nil-/xit').Xit */
    const { binding, listeners } = getContext("nil.xit");

    const int_binding = binding.number('tagged_bind', 1101);
    const string_listener = listeners.string('tagged_listen');

    const click = () => {
        int_binding.update(v => v + 1);
        string_listener(`${$int_binding} published`);
    };
</script>

<button onclick={click}>tagged {$int_binding}</button>