package com.bryanrady.douyin.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;

import androidx.appcompat.widget.AppCompatTextView;

public class VideoRecordButton extends AppCompatTextView {

    private OnRecordListener mOnRecordListener;

    public VideoRecordButton(Context context) {
        this(context,null);
    }

    public VideoRecordButton(Context context, AttributeSet attrs) {
        this(context,attrs,0);
    }

    public VideoRecordButton(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (mOnRecordListener == null){
            return false;
        }
        switch (event.getAction()){
            case MotionEvent.ACTION_DOWN:
                setPressed(true);
                mOnRecordListener.onRecordStart();
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                setPressed(false);
                mOnRecordListener.onRecordStop();
                break;
        }
        return true;
    }

    public void setOnRecordListener(OnRecordListener mListener) {
        this.mOnRecordListener = mListener;
    }

    public interface OnRecordListener{
        void onRecordStart();
        void onRecordStop();
    }

}
