<script>
    import Range from "$components/Range.svelte";
    import Text from "$components/Text.svelte";

    import { xit, json_string } from "@nil-/xit";

    const { values, signals } = xit();

    const int_value = values.number('value_0_0', 1101);
    const str_value = values.string('value_0_1', "world");

    const signal1 = signals.none("signal-1");
    const signal2 = signals.json("signal-2", json_string.encode);
    const signal3 = signals.boolean("signal-3");
    let flag = true;

    const click = () => {
        signal1();
        signal2({ value_str: $str_value, value_int: $int_value });
        signal3(flag = !flag);
    };
</script>

<button onclick={click}>hello world</button>
<Text bind:value={$str_value} placeholder="placeholder" label={"text label here"} ></Text>
<Range bind:value={$int_value} min={0} max={10} step={1} label="label here"></Range>