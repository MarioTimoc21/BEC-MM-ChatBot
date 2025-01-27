from flask import Flask, request, jsonify
from sentence_transformers import SentenceTransformer, util
import google.generativeai as genai
import json

# Configure the Gemini API
api_key = ""
genai.configure(api_key=api_key)

# Function to load and preprocess JSON context
def load_json(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    contexts = [entry["context"] for entry in data["data"]]
    return contexts

# Function to split text into manageable segments
def split_text(text, max_length=500):
    words = text.split()
    segments = []
    current_segment = []
    for word in words:
        current_segment.append(word)
        if len(" ".join(current_segment)) > max_length:
            segments.append(" ".join(current_segment))
            current_segment = []
    if current_segment:
        segments.append(" ".join(current_segment))
    return segments

# Function to create index for searching context
def create_index(segments):
    model = SentenceTransformer("all-MiniLM-L6-v2")
    embeddings = model.encode(segments, convert_to_tensor=True)
    return model, embeddings

# Function to find the most relevant segment for a query
def find_relevant_segment(query, model, embeddings, segments):
    query_embedding = model.encode(query, convert_to_tensor=True)
    scores = util.cos_sim(query_embedding, embeddings)[0]
    best_idx = scores.argmax().item()
    return segments[best_idx]

# Function to call Gemini API with a query and context
def get_gemini_response(user_input, context):
    prompt = f"Context: {context}\nQuestion: {user_input}"
    model = genai.GenerativeModel("gemini-1.5-flash")
    response = model.generate_content(
        prompt,
        generation_config=genai.GenerationConfig(
            max_output_tokens=1000,
            temperature=0.7
        )
    )
    return response.text

# Load context from JSON
contexts = load_json("modele_cleaned.json")
full_context = " ".join(contexts)
segments = split_text(full_context, max_length=500)
model, embeddings = create_index(segments)

app = Flask(__name__)

@app.route('/chat', methods=['POST'])
def chat():
    data = request.json
    user_question = data.get("question", "")

    if not user_question:
        return jsonify({"error": "No question provided"}), 400

    # Find relevant context
    relevant_segment = find_relevant_segment(user_question, model, embeddings, segments)

    # Use module context if available; otherwise, fallback to general knowledge
    if relevant_segment.strip():  # If relevant context is found
        title = "Based on Modules"
        context = relevant_segment
    else:  # Fallback to general model knowledge
        title = "Based on Model Knowledge"
        context = "General Knowledge"

    # Generate the response
    response = get_gemini_response(user_question, context)

    return jsonify({
        "title": title,
        "response": response
    })


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
