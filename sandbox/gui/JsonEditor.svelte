<script>
    import { createJSONEditor } from 'vanilla-jsoneditor/standalone.js'
    import { xit, codec_json_from_string } from "@nil-/xit";

    const { values } = xit();

    const buf_value = values('json_value', {}, codec_json_from_string);

    const json_editor = (target) => {
        let is_notified_from_store = false;
        let is_edited_from_editor = false;
        const editor = createJSONEditor({
            target,
            props: {
                content: { json: $buf_value },
                onChange: (updatedContent) => {
                    is_edited_from_editor = true;
                    if ("json" in updatedContent)
                    {
                        if (!is_notified_from_store)
                        {
                            $buf_value = updatedContent.json;
                        }
                    }
                    else if ("text" in updatedContent)
                    {
                        try
                        {
                            if (!is_notified_from_store)
                            {
                                $buf_value = JSON.parse(updatedContent.text);
                            }
                        }
                        catch (e)
                        {
                            console.log(e);
                        }
                    }
                    is_edited_from_editor = false;
                }
            }
        });
        const unsub = buf_value.subscribe((v) => {
            is_notified_from_store = true;
            if (!is_edited_from_editor) {
                editor.set({ json: v });
            }
            is_notified_from_store = false;
        });
        return {
            destroy: () => {
                unsub();
                editor.destroy();
            }
        };
    };
</script>

<div style:display="contents" use:json_editor></div>