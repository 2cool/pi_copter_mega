package cc.dewdrop.ffplayer;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import cc.dewdrop.ffplayer.utils.FFUtils;
import cc.dewdrop.ffplayer.widget.FFVideoView;

public class MainActivity extends Activity {

    private TextView mTextView;
    private FFVideoView mVideoView;




    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    private void hideSystemUI() {
        // Enables regular immersive mode.
        // For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
        // Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE
                        // Set the content to appear under the system bars so that the
                        // content doesn't resize when the system bars hide and show.
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }










    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        mTextView = findViewById(R.id.sample_text);
        mVideoView = findViewById(R.id.videoView);
    }

    public void onButtonClick(View view) {
        int id = view.getId();

        switch (id) {
            case R.id.button_protocol:
                setInfoText(FFUtils.urlProtocolInfo());
                break;
            case R.id.button_codec:
                setInfoText(FFUtils.avCodecInfo());
                break;
            case R.id.button_filter:
                setInfoText(FFUtils.avFilterInfo());
                break;
            case R.id.button_format:
                setInfoText(FFUtils.avFormatInfo());
                break;
            case R.id.button_play:
                String videoPath = Environment.getExternalStorageDirectory() + "/Movies/PERU.MP4";
                mVideoView.playVideo(videoPath);
                break;
        }
    }

    private void setInfoText(String content) {
        if (mTextView != null) {
            mTextView.setText(content);
        }
    }
}
