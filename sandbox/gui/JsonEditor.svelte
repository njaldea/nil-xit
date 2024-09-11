<script>
    import { binding } from "@nil-/xit";

    import { JSONEditor } from 'vanilla-jsoneditor/standalone.js'

    const buf_binding = binding('json_binding', {});

    const json_editor = (d) => {
        const editor = new JSONEditor({
            target: d,
            props: {
                content: { json: $buf_binding },
                onChange: (updatedContent, previousContent, { contentErrors, patchResult }) => {
                    console.log(updatedContent);
                    if (updatedContent.json)
                    {
                        $buf_binding = updatedContent.json   
                    }
                    else if (updatedContent.text)
                    {
                        try
                        {
                            $buf_binding = JSON.parse(updatedContent.text)
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

<div use:json_editor/>
