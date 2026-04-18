<script>
    import Range from "$components/Range.svelte";
    import Text from "$components/Text.svelte";

    import { xit, codec_number, codec_json_from_string, codec_string, codec_bool } from "@nil-/xit";

    const { values, signals } = xit();

    const int_value = values('value_0_0', 1101, codec_number);
    const str_value = values('value_0_1', "world", codec_string);

    const signal1 = signals("signal-1");
    const signal2 = signals("signal-2", codec_json_from_string.encode);
    const signal3 = signals("signal-3", codec_bool.encode);
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