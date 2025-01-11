from huggingface_hub import InferenceClient

client = InferenceClient(api_key="")
while True:
    mesaj = input("User: ")
    if mesaj.lower() in ["exit", "quit", "bye"]:
        break

    messages = [
        {
            "role": "user",
            "content": mesaj
        }
    ]

    completion = client.chat.completions.create(
        model="microsoft/DialoGPT-medium", 
        messages=messages, 
        max_tokens=500
    )

    print(f"Bot: {completion.choices[0].message.content}")