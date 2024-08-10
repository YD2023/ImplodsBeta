#GUI using chess library and flask

from flask import Flask
from flask import render_template
from flask import request
import chess
import chess.engine

engine = chess.engine.SimpleEngine.popen_uci("C:\ImplodsBeta\EngineFiles\python_HTML frontend\engine\ImplodsBeta21.exe")


app = Flask (__name__)

#define root
@app.route('/')

def root():
    return render_template('UI.html')

#API for making moves
@app.route('/make_move',methods=['POST'])
def make_move():
    #get FEN from HTTP
    fen = request.form.get('fen')
    board = chess.Board(fen) 

    result = engine.play(board, chess.engine.Limit(time=5))
    board.push(result.move)
    fen = board.fen()
    return {'fen': fen}

#main
if __name__ == '__main__':
    #start server
    app.run(debug=True, threaded=True)