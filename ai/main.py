from flask import Flask, request, jsonify
from huggingface_hub import InferenceClient

# Initialize Flask app and Hugging Face client
app = Flask(__name__)
client = InferenceClient(api_key="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")  # Replace with your API key

@app.route('/chat', methods=['POST'])
def chat():
    try:
        # Get user input from the request
        user_input = request.json.get('message', '')
        if not user_input:
            return jsonify({"error": "No message provided"}), 400

        # Generate chatbot response using Hugging Face API
        messages = [{"role": "user", "content": user_input}]
        completion = client.chat.completions.create(
            model="microsoft/DialoGPT-medium",
            messages=messages,
            max_tokens=500
        )
        bot_response = completion.choices[0].message.content

        # Return the bot's response as JSON
        return bot_response

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
