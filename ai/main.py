from flask import Flask, request, jsonify
from sentence_transformers import SentenceTransformer, util
import openai
import json


openai.api_key = "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"

def load_json(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    contexts = [entry["context"] for entry in data["data"]]
    return contexts

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

def create_index(segments):
    model = SentenceTransformer("all-MiniLM-L6-v2")
    embeddings = model.encode(segments, convert_to_tensor=True)
    return model, embeddings

def find_relevant_segment(query, model, embeddings, segments):
    query_embedding = model.encode(query, convert_to_tensor=True)
    scores = util.cos_sim(query_embedding, embeddings)[0]
    best_idx = scores.argmax().item()
    return segments[best_idx]

def get_response(user_input, context):
    messages = [
        {"role": "system", "content": "You are an expert capable of answering questions based on the given modules."},
        {"role": "user", "content": f"Context: {context}\nQuestion: {user_input}"}
    ]
    response = openai.ChatCompletion.create(
        model="gpt-3.5-turbo",
        messages=messages,
        max_tokens=150,
        temperature=0.7
    )
    return response.choices[0].message['content']

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

    relevant_segment = find_relevant_segment(user_question, model, embeddings, segments)

    response = get_response(user_question, relevant_segment)

    return response

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)