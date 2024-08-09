#GUI using chess library and flask

from flask import Flask
from flask import render_template

app = Flask (__name__)

#define root
@app.route('/')

def root():
    return render_template('UI.html')

#main
if __name__ == '__main__':
    #start server
    app.run(debug=True, threaded=True)