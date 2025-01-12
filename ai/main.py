from flask import Flask, request, jsonify
import requests

# Hugging Face API details
API_URL = "https://api-inference.huggingface.co/models/deepset/roberta-base-squad2"
HEADERS = {"Authorization": "Bearer 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"}  # Replace with your Hugging Face API key

# Context for the chatbot
CONTEXT = """
Types and formats of digital content: Digital content encompasses a wide range of information and media that is created and distributed in electronic format; it can be: Text-based content (Articles, Blog posts, eBooks, Whitepapers), Visual content (Images, Infographics, Illustrations, Photos, Memes), Audio content (Podcasts, Music, Audiobooks, Sound effects, Voice-overs), Video content (Short videos, Explainer videos, Tutorials, Webinars), Social media content (Social media posts, Memes, GIFs, Stories, Videos), etc
AI Generated Content: AI-generated content refers to digital content that is created with the assistance of artificial intelligence (AI) technologies. It involves using algorithms and machine learning models to automate the process of generating written, visual, or audio content.
Accessibility incorporation in digital content: Incorporating accessibility in digital content means ensuring that the content is designed and developed in a way that allows people with disabilities or diverse needs to access and interact with it effectively. The goal is to provide equal access and a positive user experience for all individuals, regardless of their abilities.
Virtual reality, augmented reality and mixed reality: there are 3 technologies that enhance the user's perception and interaction with the digital world. While they share similarities, they differ in the way they merge digital content with the real world.
Digital content on personal, professional and open platforms: The choice of platform depends on the purpose and intended audience of the digital content. Personal platforms provide a space for self-expression and social connections, professional platforms cater to business or career-related goals, while open platforms foster collaboration and the sharing of knowledge and resources. It is common for individuals and organizations to utilize multiple platforms to reach different audiences and achieve different objectives.
Data visualisation, Data manipulation, Data attribution: In summary, data visualization enables the effective presentation and interpretation of data through visual representations. Data manipulation involves transforming and manipulating data to derive insights or meet specific requirements. Data attribution ensures proper crediting and acknowledgment of data sources in digital content creation. These three aspects are essential components in working with data and creating meaningful and impactful digital content.
SEO and digital marketing: SEO and digital marketing are key components of online business strategy. SEO focuses on improving a website's visibility on search engines through techniques like keyword research and link building. Digital marketing is broader, including not only SEO but also paid advertising, social media, and email marketing. Both aim to attract and engage a target audience online, but digital marketing additionally focuses on conversions and sales. Together, they form a synergistic approach for enhancing online visibility, brand awareness, and revenue.
Protection techniques and mechanisms for copywriting: It is important to note that while the protection techniques and mechanisms can deter unauthorized use and distribution of digital content, they may not provide fool-proof security.
Licences: proprietary, free and open-source: The choice of license depends on the content owner's goals and intentions for their digital content. Proprietary licenses are commonly used when commercial interests are involved and provide the owner with exclusive control over the content. Free and open-source licenses, on the other hand, promote sharing, collaboration, and community-driven development. These licenses allow users to freely use, modify, and distribute digital content, fostering innovation and creativity within the community.
Classical programming: Classical / software programming, also known as coding or software development, is the process of creating computer programs or software applications. It involves writing instructions or code using programming languages to tell a computer what to do.
AI Programming, general: AI programming, also known as artificial intelligence programming, involves developing software or algorithms that enable machines or computer systems to mimic human intelligence and perform tasks that typically require human intelligence.
AI Programming, ethics: AI programming ethics refers to the ethical considerations and principles that guide the development, deployment, and use of artificial intelligence (AI) systems. As AI technology becomes more advanced and pervasive, there is a growing recognition of the need to ensure that AI systems are designed and used in an ethical and responsible manner.
"""

# Initialize Flask app
app = Flask(__name__)

@app.route('/chat', methods=['POST'])
def chat():
    try:
        # Get the user's question from the request
        data = request.json
        question = data.get("question", "")
        if not question:
            return jsonify({"error": "Question is required"}), 400

        # Query the Hugging Face API
        payload = {"inputs": {"question": question, "context": CONTEXT}}
        response = requests.post(API_URL, headers=HEADERS, json=payload)

        # Parse the API response
        if response.status_code == 200:
            return response.json().get("answer", "No answer found")
        else:
            return jsonify({"error": f"API error: {response.status_code}, {response.text}"}), response.status_code

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
