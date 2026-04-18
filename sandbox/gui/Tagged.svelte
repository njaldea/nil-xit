<script>
    import { xit, json_string } from "@nil-/xit";

    const { values, signals } = xit();
    
    const int_value = values.number('tagged_value', 1101);
    const json_value = values.json('tagged_json', {}, json_string);
    const string_signal = signals.string('tagged_signal');

    const click = () => {
        int_value.update(v => v + 1);
        string_signal(`${$int_value} published`);
    };
</script>

<button onclick={click}>tagged {$int_value} {JSON.stringify($json_value)}</button>