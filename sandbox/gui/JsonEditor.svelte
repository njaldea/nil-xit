<script>
    import { createJSONEditor } from 'vanilla-jsoneditor/standalone.js'
    import { xit, json_string } from "@nil-/xit";

    const { values } = xit();

    const buf_value = values.json('json_value', {}, json_string);

    const json_editor = (d) => {
        const editor = createJSONEditor({
            target: d,
            props: {
                content: { json: $buf_value },
                onChange: (updatedContent, previousContent, { contentErrors, patchResult }) => {
                    if (updatedContent.json)
                    {
                        $buf_value = updatedContent.json   
                    }
                    else if (updatedContent.text)
                    {
                        try
                        {
                            $buf_value = JSON.parse(updatedContent.text)
                        }
                        catch (e)
                        {
                            console.log(e);
                        }
                    }
                }
            }
        });
        return { destroy: () => editor.destroy() };
    };
</script>

<div style="display: contents" use:json_editor></div>
