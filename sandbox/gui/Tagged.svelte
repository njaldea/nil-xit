<script>
    import { xit, codec_number, codec_string, codec_json_from_string } from "@nil-/xit";

    const { values, signals } = xit();
    
    const int_value = values('tagged_value', 1101, codec_number);
    const json_value = values('tagged_json', {}, codec_json_from_string);
    const string_signal = signals('tagged_signal', codec_string);

    const click = () => {
        int_value.update(v => v + 1);
        string_signal(`${$int_value} published`);
    };
</script>

<button onclick={click}>tagged {$int_value} {JSON.stringify($json_value)}</button>