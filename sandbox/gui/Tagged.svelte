<script>
    import { xit } from "@nil-/xit";

    const { values, signals } = xit();

    const int_value = values.number('tagged_value', 1101);
    const string_signal = signals.string('tagged_signal');

    const click = () => {
        int_value.update(v => v + 1);
        string_signal(`${$int_value} published`);
    };
</script>

<button onclick={click}>tagged {$int_value}</button>