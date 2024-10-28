<script>
    import { xit } from "@nil-/xit";

    const { signals } = xit();
    const send = signals.string("message");
    console.log(send)
    let command = "";
</script>

<div>
    <input bind:value={command}/>
    <button onclick={() => {
        console.log(command)
        send(command)
    }}>SEND</button>
</div>

<style>
    div
    {
        display: flex;
        flex-direction: column;
    }
</style>