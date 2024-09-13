<script>
    // can import local files
    import Range from "./components/Range.svelte";
    import Text from "./components/Text.svelte";

    import { json_string } from "@nil-/xit";
    import { getContext } from "svelte";

    /** @type import('./nil-xit').Xit */
    const { binding, listeners } = getContext("nil.xit");

    // (binding id, default value) (in case binding is not registered)
    const int_binding = binding.number('binding_0_0', 1101);
    const str_binding = binding.string('binding_0_1', "world");

    const handler = listeners.none("listener-1");
    const handler2 = listeners.json("listener-2", json_string.encode);
    const handler3 = listeners.boolean("listener-3");
    let flag = true;

    const click = () => {
        handler();
        handler2({ value_str: $str_binding, value_int: $int_binding });
        handler3(flag = !flag);
    };
</script>

<button onclick={click}>hello world</button>
<Text bind:value={$str_binding} placeholder="placeholder" label={"text label here"} ></Text>
<Range bind:value={$int_binding} min={0} max={10} step={1} label="label here"></Range>